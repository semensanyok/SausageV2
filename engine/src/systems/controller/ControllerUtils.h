#pragma once

namespace ControllerUtils {
inline bool IsInScreenBorders(float screen_x, float screen_y) {
    return screen_x > 0 || screen_x < GameSettings::SCR_WIDTH || screen_y > 0 || screen_y < GameSettings::SCR_HEIGHT;
}
}
