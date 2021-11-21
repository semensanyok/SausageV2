#include "ScreenOverlayManager.h"

bool operator==(const Point& p1, const Point& p2) {
    return p1.x == p1.x && p1.y == p2.y; 
};
std::ostream& operator<<(std::ostream& in, const Point& pt) {
  in << "(" << pt.x << "," << pt.y << ")";
  return in;
};
