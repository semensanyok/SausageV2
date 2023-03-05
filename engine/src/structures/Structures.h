#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

struct Point {
  int x;
  int y;
};

inline std::ostream& operator<<(std::ostream& in, const Point& pt) {
  in << "(" << pt.x << "," << pt.y << ")";
  return in;
};
inline bool operator==(const Point& p1, const Point& p2) {
  return p1.x == p1.x && p1.y == p2.y;
};
template <>
struct std::formatter<Point> : std::formatter<std::string> {
  auto format(Point p, format_context& ctx) {
    return formatter<string>::format(
      std::format("(x={}, y={})", p.x, p.y), ctx);
  }
};
