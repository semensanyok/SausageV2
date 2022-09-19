#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

class BufferStorage;
class BufferConsumer;

inline std::ostream& operator<<(std::ostream& in, const Point& pt){
  in << "(" << pt.x << "," << pt.y << ")";
  return in;
}
inline bool operator==(const Point& p1, const Point& p2) {
    return p1.x == p1.x && p1.y == p2.y; 
}
template <>
struct std::formatter<Point> : std::formatter<std::string> {
  auto format(Point p, format_context& ctx) {
    return formatter<string>::format(
      std::format("(x={}, y={})", p.x, p.y), ctx);
  }
};

namespace AnchorRelativeToNodePosition {
  enum AnchorRelativeToNodePosition {
    LeftBottom,
    LeftTop,
    RightBottom,
    RightTop,
  };
}

struct UINodePosition {
  Point anchor_position;
  AnchorRelativeToNodePosition::AnchorRelativeToNodePosition anchor_relative_to_node_position;
  int width;
  int height;
};

