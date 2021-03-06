#include <fc/fc.hpp>

namespace higan::Famicom {

Interface* interface = nullptr;

auto FamicomInterface::game() -> string {
  if(fds.node) {
    return fds.name();
  }

  if(cartridge.node) {
    return cartridge.name();
  }

  return "(no cartridge connected)";
}

auto FamicomInterface::root() -> Node::Object {
  return system.node;
}

auto FamicomInterface::load(Node::Object& root, string tree) -> void {
  interface = this;
  system.load(root, Node::unserialize(tree));
}

auto FamicomInterface::unload() -> void {
  system.unload();
}

auto FamicomInterface::save() -> void {
  system.save();
}

auto FamicomInterface::power() -> void {
  system.power(false);
}

auto FamicomInterface::run() -> void {
  system.run();
}

auto FamicomInterface::serialize(bool synchronize) -> serializer {
  return system.serialize(synchronize);
}

auto FamicomInterface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

}
