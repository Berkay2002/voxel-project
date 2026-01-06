#include "VideoSettings.h"

namespace Core {

VideoSettings &VideoSettings::Instance() {
  static VideoSettings instance;
  return instance;
}

} // namespace Core
