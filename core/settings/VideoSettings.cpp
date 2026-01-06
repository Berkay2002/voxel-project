#include "VideoSettings.h"

namespace Core {

DisplayConfig &DisplayConfig::Instance() {
  static DisplayConfig instance;
  return instance;
}

} // namespace Core
