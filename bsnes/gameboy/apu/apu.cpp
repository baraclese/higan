#include <gameboy/gameboy.hpp>

#define APU_CPP
namespace GameBoy {

#include "square1/square1.cpp"
#include "square2/square2.cpp"
#include "wave/wave.cpp"
#include "noise/noise.cpp"
#include "master/master.cpp"
#include "serialization.cpp"
APU apu;

void APU::Main() {
  apu.main();
}

void APU::main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if((counter & 8191) == 0) {  //512hz
      if(sequencer == 0 || sequencer == 2 || sequencer == 4 || sequencer == 6) {  //256hz
        square1.clock_length();
        square2.clock_length();
        wave.clock_length();
        noise.clock_length();
      }
      if(sequencer == 2 || sequencer == 6) {  //128hz
        square1.clock_sweep();
      }
      if(sequencer == 7) {  //64hz
        square1.clock_envelope();
        square2.clock_envelope();
        noise.clock_envelope();
      }
      sequencer = (sequencer + 1) & 7;
    }

    square1.run();
    square2.run();
    wave.run();
    noise.run();
    master.run();

    system.interface->audio_sample(master.center, master.left, master.right);

    if(++counter == 4194304) counter = 0;

    if(++clock >= 0) co_switch(scheduler.active_thread = cpu.thread);
  }
}

void APU::power() {
  create(Main, 4194304);
  for(unsigned n = 0xff10; n <= 0xff3f; n++) bus.mmio[n] = this;

  foreach(n, mmio_data) n = 0x00;
  counter = 0;
  sequencer = 0;

  square1.power();
  square2.power();
  wave.power();
  noise.power();
  master.power();
}

uint8 APU::mmio_read(uint16 addr) {
  static const uint8 table[48] = {
    0x80, 0x3f, 0x00, 0xff, 0xbf,                          //square1
    0xff, 0x3f, 0x00, 0xff, 0xbf,                          //square2
    0x7f, 0xff, 0x9f, 0xff, 0xbf,                          //wave
    0xff, 0xff, 0x00, 0x00, 0xbf,                          //noise
    0x00, 0x00, 0x70,                                      //master
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //unmapped
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        //wave pattern
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        //wave pattern
  };

  if(addr == 0xff26) {
    uint8 data = master.enable << 7;
    if(square1.counter && square1.length) data |= 0x01;
    if(square2.counter && square2.length) data |= 0x02;
    if(   wave.counter &&    wave.length) data |= 0x04;
    if(  noise.counter &&   noise.length) data |= 0x08;
    return data | table[addr - 0xff10];
  }

  if(addr >= 0xff10 && addr <= 0xff3f) return mmio_data[addr - 0xff10] | table[addr - 0xff10];
  return 0xff;
}

void APU::mmio_write(uint16 addr, uint8 data) {
  if(addr >= 0xff10 && addr <= 0xff3f) mmio_data[addr - 0xff10] = data;

  if(addr >= 0xff10 && addr <= 0xff14) return square1.write        (addr - 0xff10, data);
  if(addr >= 0xff15 && addr <= 0xff19) return square2.write        (addr - 0xff15, data);
  if(addr >= 0xff1a && addr <= 0xff1e) return    wave.write        (addr - 0xff1a, data);
  if(addr >= 0xff1f && addr <= 0xff23) return   noise.write        (addr - 0xff1f, data);
  if(addr >= 0xff24 && addr <= 0xff26) return  master.write        (addr - 0xff24, data);
  if(addr >= 0xff30 && addr <= 0xff3f) return    wave.write_pattern(addr - 0xff30, data);
}

}