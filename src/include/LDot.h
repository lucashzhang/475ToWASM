#ifndef LDOTDEF
#define LDOTDEF

#include "GMath.h"
#include "LUtil.h"
#include <vector>
#include <cmath>

struct LDot
{
  int x;
  int w;

  LDot(int nX, int nW) : x(nX), w(nW) {}
};

static inline void LEdgeToDots(std::vector<std::vector<LDot>> &dots, const GPoint &p0, const GPoint &p1, const GIRect &bounds)
{
  int numDots = std::abs(CLAMP(GRoundToInt(p0.y()), bounds.top(), bounds.bottom()) - CLAMP(GRoundToInt(p1.y()), bounds.top(), bounds.bottom()));
  if (numDots == 0)
    return;

  float dx = (p1.x() - p0.x()) / (p1.y() - p0.y());
  float b = p0.x() - dx * p0.y();
  int y = CLAMP(GRoundToInt(std::min(p0.y(), p1.y())), bounds.top(), bounds.bottom());
  float x = dx * (y + 0.5) + b;
  int winding = p0.y() < p1.y() ? 1 : -1;

  for (int i = 0; i < numDots; ++i, x += dx, ++y)
  {
    dots[y].emplace_back(CLAMP(GRoundToInt(x), bounds.left(), bounds.right()), winding);
  }
}

static inline float quadError(const GPoint &p0, const GPoint &p1, const GPoint &p2)
{
  const GPoint e = (-1 * p0 + 2 * p1 - p2) * 0.25;
  return std::sqrt(e.x() * e.x() + e.y() * e.y());
}

static inline float cubicError(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPoint &p3)
{
  const GPoint a = -1 * p0 + 2 * p1 + -1 * p2;
  const GPoint b = -1 * p1 + 2 * p2 + -1 * p3;
  float eX = std::max(std::abs(a.x()), std::abs(b.x()));
  float eY = std::max(std::abs(a.y()), std::abs(b.y()));
  return std::sqrt(eX * eX + eY * eY);
}

static inline void LQuadToDots(std::vector<std::vector<LDot>> &dots, const GPoint &p0, const GPoint &p1, const GPoint &p2, const GIRect &bounds)
{
  GPoint a = QUADA(p0, p1, p2);
  GPoint b = QUADB(p0, p1);
  int numSegments = GCeilToInt(2 * std::sqrt(quadError(p0, p1, p2)));
  float dt = 1.0f / numSegments;
  GPoint prevPoint = p0;
  float t = dt;
  for (int i = 1; i < numSegments; ++i, t += dt)
  {
    GPoint currPoint = HORNER2(a, b, p0, t);
    LEdgeToDots(dots, prevPoint, currPoint, bounds);
    prevPoint = GPoint(currPoint);
  }
  LEdgeToDots(dots, prevPoint, p2, bounds);
}

static inline void LCubicToDots(std::vector<std::vector<LDot>> &dots, const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPoint &p3, const GIRect &bounds)
{
  GPoint a = CUBICA(p0, p1, p2, p3);
  GPoint b = CUBICB(p0, p1, p2);
  GPoint c = CUBICC(p0, p1);
  int numSegments = GCeilToInt(std::sqrt(3 * cubicError(p0, p1, p2, p3)));
  float dt = 1.0f / numSegments;
  GPoint prevPoint = p0;
  float t = dt;
  for (int i = 1; i < numSegments; ++i, t += dt)
  {
    GPoint currPoint = HORNER3(a, b, c, p0, t);
    LEdgeToDots(dots, prevPoint, currPoint, bounds);
    prevPoint = GPoint(currPoint);
  }
  LEdgeToDots(dots, prevPoint, p3, bounds);
}

static inline void LPathToDots(std::vector<std::vector<LDot>> &dots, const GPath &path, const GIRect &bounds)
{
  // int numDots = 0;
  GPath::Edger edger(path);
  std::vector<GPoint> pts(GPath::kMaxNextPoints);

  auto verb = edger.next(pts.data());
  while (verb != GPath::Verb::kDone)
  {
    switch (verb)
    {
    case GPath::Verb::kLine:
      // ctm.mapPoints(pts.data(), 2);
      LEdgeToDots(dots, pts[0], pts[1], bounds);
      break;
    case GPath::Verb::kQuad:
      LQuadToDots(dots, pts[0], pts[1], pts[2], bounds);
      break;
    case GPath::Verb::kCubic:
      LCubicToDots(dots, pts[0], pts[1], pts[2], pts[3], bounds);
      break;
    default:
      break;
    }
    verb = edger.next(pts.data());
  }
}

#endif