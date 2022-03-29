#ifndef LUTILDEF
#define LUTILDEF

#include "GPoint.h"
#include "GRect.h"
#include <cmath>

#define CLAMP(a, x, y) (std::max(x, std::min(a, y)))
#define LERP(p0, p1, t) ((1 - t) * p0 + t * p1)
#define BILERP(x, y, a, b, c, d) ((1-x) * (1-y) * a) + (x * (1 - y) * b) + (x * y * c) + ((1 - x) * y * d)
#define HORNER2(a, b, c, x) (((a * x) + b) * x) + c
#define HORNER3(a, b, c, d, x) (HORNER2(a, b, c, x)) * x + d
#define CUBICA(a, b, c, d) -1 * a + 3 * b + -3 * c + d
#define CUBICB(a, b, c) 3 * a + -6 * b + 3 * c
#define CUBICC(a, b) -3 * a + 3 * b
#define QUADA(a, b, c) a + -2 * b + c
#define QUADB(a, b) -2 * a + 2 * b

namespace PathUtil
{
  static inline GRect unite(GRect &r0, GRect &r1)
  {
    float left = std::min(r0.left(), r1.left());
    float top = std::min(r0.top(), r1.top());
    float right = std::max(r0.right(), r1.right());
    float bottom = std::max(r0.bottom(), r1.bottom());
    return GRect::MakeLTRB(left, top, right, bottom);
  }

  static inline GRect lineBounds(GPoint &p0, GPoint &p1)
  {
    float left = std::min(p0.x(), p1.x());
    float top = std::min(p0.y(), p1.y());
    float right = std::max(p0.x(), p1.x());
    float bottom = std::max(p0.y(), p1.y());
    return GRect::MakeLTRB(left, top, right, bottom);
  }

  static inline GRect quadBounds(GPoint &p0, GPoint &p1, GPoint &p2)
  {
    // t = (a - b) / (c - 2b + a)
    GPoint a = QUADA(p0, p1, p2);
    GPoint b = QUADB(p0, p1);
    GRect bounds = lineBounds(p0, p2);
    float tx = (p0.x() - p1.x()) / (p2.x() - 2 * p1.x() + p0.x());
    float ty = (p0.y() - p1.y()) / (p2.y() - 2 * p1.y() + p0.y());
    if (tx > 0 && tx < 1)
    {
      GPoint pt = HORNER2(a, b, p0, tx);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }
    if (ty > 0 && ty < 1)
    {
      GPoint pt = HORNER2(a, b, p0, ty);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }
    return bounds;
  }

  static inline GRect cubicBounds(GPoint &p0, GPoint &p1, GPoint &p2, GPoint &p3)
  {
    GPoint a = CUBICA(p0, p1, p2, p3);
    GPoint b = CUBICB(p0, p1, p2);
    GPoint c = CUBICC(p0, p1);
    GPoint i = (-3 * p0) + (9 * p1) + (-9 * p2) + (3 * p3);
    GPoint j = (6 * p0) + (-12 * p1) + (6 * p2);
    GPoint k = (-3 * p0) + (3 * p1);
    float tx0 = 0;
    float tx1 = 0;
    float ty0 = 0;
    float ty1 = 0;
    if (i.x() != 0)
    {
      tx0 = (-j.x() + std::sqrt(j.x() * j.x() - 4 * i.x() * k.x())) / (2 * i.x());
      tx1 = (-j.x() - std::sqrt(j.x() * j.x() - 4 * i.x() * k.x())) / (2 * i.x());
    }
    if (i.y() != 0)
    {
      ty0 = (-j.y() + std::sqrt(j.y() * j.y() - 4 * i.y() * k.y())) / (2 * i.y());
      ty1 = (-j.y() - std::sqrt(j.y() * j.y() - 4 * i.y() * k.y())) / (2 * i.y());
    }
    GRect bounds = lineBounds(p0, p3);
    if (tx0 > 0 && tx0 < 1)
    {
      GPoint pt = HORNER3(a, b, c, p0, tx0);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }
    if (ty0 > 0 && ty0 < 1)
    {
      GPoint pt = HORNER3(a, b, c, p0, ty0);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }
    if (tx1 > 0 && tx1 < 1)
    {
      GPoint pt = HORNER3(a, b, c, p0, tx1);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }
    if (ty1 > 0 && ty1 < 1)
    {
      GPoint pt = HORNER3(a, b, c, p0, ty1);
      bounds.fLeft = std::min(bounds.fLeft, pt.x());
      bounds.fRight = std::max(bounds.fRight, pt.x());
      bounds.fTop = std::min(bounds.fTop, pt.y());
      bounds.fBottom = std::max(bounds.fBottom, pt.y());
    }

    return bounds;
  }
}

#endif