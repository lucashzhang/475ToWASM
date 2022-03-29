#include "GPath.h"
#include "GPoint.h"
#include "GRect.h"
#include "GMatrix.h"
#include "LUtil.h"
#include <vector>
#include <cmath>

GPath &GPath::addRect(const GRect &rect, Direction dir)
{
  moveTo(rect.left(), rect.top());
  switch (dir)
  {
  case GPath::Direction::kCCW_Direction:
    lineTo(rect.left(), rect.bottom());
    lineTo(rect.right(), rect.bottom());
    lineTo(rect.right(), rect.top());
    break;
  default:
    lineTo(rect.right(), rect.top());
    lineTo(rect.right(), rect.bottom());
    lineTo(rect.left(), rect.bottom());
    break;
  }
  return *this;
}

GPath &GPath::addPolygon(const GPoint pts[], int count)
{
  moveTo(GPoint(pts[0]));
  for (int i = 1; i < count; ++i)
  {
    lineTo(GPoint(pts[i]));
  }
  return *this;
}

GRect GPath::bounds() const
{
  if (fPts.size() == 0)
  {
    return GRect::MakeLTRB(0, 0, 0, 0);
  }
  GPath::Edger edger(*this);
  Verb v;
  std::vector<GPoint> pts(GPath::kMaxNextPoints);
  GRect outerBounds = GRect::MakeXYWH(fPts[0].x(), fPts[0].y(), 0, 0);
  while ((v = edger.next(pts.data())) != GPath::kDone)
  {
    GRect innerBounds;
    switch (v)
    {
    case GPath::kLine:
      innerBounds = PathUtil::lineBounds(pts[0], pts[1]);
      break;
    case GPath::kQuad:
      innerBounds = PathUtil::quadBounds(pts[0], pts[1], pts[2]);
      break;
    case GPath::kCubic:
      innerBounds = PathUtil::cubicBounds(pts[0], pts[1], pts[2], pts[3]);
      break;
    default:
      break;
    }
    outerBounds = PathUtil::unite(outerBounds, innerBounds);
  }
  return outerBounds;
}

void GPath::transform(const GMatrix &m)
{
  m.mapPoints(fPts.data(), countPoints());
}

// GPath &GPath::addCircle(GPoint center, float radius, Direction dir)
// {
//   float c = 0.551915024494;
//   GMatrix transform = GMatrix::Translate(center.x(), center.y()) * GMatrix::Scale(radius, radius);
//   GPoint start = {1, 0};
//   moveTo(transform * start);
//   switch (dir)
//   {
//   case GPath::Direction::kCCW_Direction:
//   {
//     GPoint cubic1[] = {{1, c}, {c, 1}, {0, 1}};
//     transform.mapPoints(cubic1, 3);
//     cubicTo(cubic1[0], cubic1[1], cubic1[2]);
//     GPoint cubic2[] = {{-c, 1}, {-1, c}, {-1, 0}};
//     transform.mapPoints(cubic2, 3);
//     cubicTo(cubic2[0], cubic2[1], cubic2[2]);
//     GPoint cubic3[] = {{-1, -c}, {-c, -1}, {0, -1}};
//     transform.mapPoints(cubic3, 3);
//     cubicTo(cubic3[0], cubic3[1], cubic3[2]);
//     GPoint cubic4[] = {{c, -1}, {1, -c}, {1, 0}};
//     transform.mapPoints(cubic4, 3);
//     cubicTo(cubic4[0], cubic4[1], cubic4[2]);
//     break;
//   }
//   default:
//   {
//     GPoint cubic1[] = {{1, -c}, {c, -1}, {0, -1}};
//     transform.mapPoints(cubic1, 3);
//     cubicTo(cubic1[0], cubic1[1], cubic1[2]);
//     GPoint cubic2[] = {{-c, -1}, {-1, -c}, {-1, 0}};
//     transform.mapPoints(cubic2, 3);
//     cubicTo(cubic2[0], cubic2[1], cubic2[2]);
//     GPoint cubic3[] = {{-1, c}, {-c, 1}, {0, 1}};
//     transform.mapPoints(cubic3, 3);
//     cubicTo(cubic3[0], cubic3[1], cubic3[2]);
//     GPoint cubic4[] = {{c, 1}, {1, c}, {1, 0}};
//     transform.mapPoints(cubic4, 3);
//     cubicTo(cubic4[0], cubic4[1], cubic4[2]);
//     break;
//   }
//   }

//   return *this;
// }

GPath &GPath::addCircle(GPoint center, float radius, Direction dir)
{
  float r = 0.70710678118;
  float h = 0.41421356237;
  GMatrix transform = GMatrix::Translate(center.x(), center.y()) * GMatrix::Scale(radius, radius);
  GPoint start = {1, 0};
  moveTo(transform * start);
  switch (dir)
  {
  case GPath::Direction::kCCW_Direction:
  {
    GPoint arc1[] = {{1, h}, {r, r}};
    transform.mapPoints(arc1, 2);
    quadTo(arc1[0], arc1[1]);
    GPoint arc2[] = {{h, 1}, {0, 1}};
    transform.mapPoints(arc2, 2);
    quadTo(arc2[0], arc2[1]);
    GPoint arc3[] = {{-h, 1}, {-r, r}};
    transform.mapPoints(arc3, 2);
    quadTo(arc3[0], arc3[1]);
    GPoint arc4[] = {{-1, h}, {-1, 0}};
    transform.mapPoints(arc4, 2);
    quadTo(arc4[0], arc4[1]);
    GPoint arc5[] = {{-1, -h}, {-r, -r}};
    transform.mapPoints(arc5, 2);
    quadTo(arc5[0], arc5[1]);
    GPoint arc6[] = {{-h, -1}, {0, -1}};
    transform.mapPoints(arc6, 2);
    quadTo(arc6[0], arc6[1]);
    GPoint arc7[] = {{h, -1}, {r, -r}};
    transform.mapPoints(arc7, 2);
    quadTo(arc7[0], arc7[1]);
    GPoint arc8[] = {{1, -h}, {1, 0}};
    transform.mapPoints(arc8, 2);
    quadTo(arc8[0], arc8[1]);
    break;
  }
  default:
  {
    GPoint arc1[] = {{1, -h}, {r, -r}};
    transform.mapPoints(arc1, 2);
    quadTo(arc1[0], arc1[1]);
    GPoint arc2[] = {{h, -1}, {0, -1}};
    transform.mapPoints(arc2, 2);
    quadTo(arc2[0], arc2[1]);
    GPoint arc3[] = {{-h, -1}, {-r, -r}};
    transform.mapPoints(arc3, 2);
    quadTo(arc3[0], arc3[1]);
    GPoint arc4[] = {{-1, -h}, {-1, 0}};
    transform.mapPoints(arc4, 2);
    quadTo(arc4[0], arc4[1]);
    GPoint arc5[] = {{-1, h}, {-r, r}};
    transform.mapPoints(arc5, 2);
    quadTo(arc5[0], arc5[1]);
    GPoint arc6[] = {{-h, 1}, {0, 1}};
    transform.mapPoints(arc6, 2);
    quadTo(arc6[0], arc6[1]);
    GPoint arc7[] = {{h, 1}, {r, r}};
    transform.mapPoints(arc7, 2);
    quadTo(arc7[0], arc7[1]);
    GPoint arc8[] = {{1, h}, {1, 0}};
    transform.mapPoints(arc8, 2);
    quadTo(arc8[0], arc8[1]);
    break;
  }
  }

  return *this;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t)
{
  dst[1] = LERP(src[0], src[1], t);
  dst[3] = LERP(src[1], src[2], t);
  dst[2] = LERP(dst[1], dst[3], t);
  dst[0] = src[0];
  dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t)
{
  dst[1] = LERP(src[0], src[1], t);
  GPoint b = LERP(src[1], src[2], t);
  dst[5] = LERP(src[2], src[3], t);
  dst[2] = LERP(dst[1], b, t);
  dst[4] = LERP(b, dst[5], t);
  dst[3] = LERP(dst[2], dst[4], t);
  dst[0] = src[0];
  dst[6] = src[3];
}
