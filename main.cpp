#include "core/Engine.h"
#include "core/Logger.h"

#include <exception>

int main() {
  try {
    Core::Engine engine;
    engine.Run();
  } catch (const std::exception &e) {
    LOG_ERROR(std::string("Fatal error: ") + e.what());
    return 1;
  }

  return 0;
}
