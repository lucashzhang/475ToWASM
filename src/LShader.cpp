#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GMath.h"
#include "GPoint.h"
#include "LUtil.h"
#include "LPainter.h"
#include <cmath>

class LShader : public GShader
{
public:
  LShader(const GBitmap &newBitmap, const GMatrix &ctm, GShader::TileMode mode) : bitmap(newBitmap), localMatrix(ctm * GMatrix::Scale(newBitmap.width(), newBitmap.height())), context(GMatrix()), invContext(GMatrix())
  {
    switch (mode)
    {
    case GShader::TileMode::kRepeat:
      tile = repeat;
      break;
    case GShader::TileMode::kMirror:
      tile = mirror;
      break;
    default:
      tile = clamp;
    }
  }

  bool isOpaque()
  {
    return bitmap.isOpaque();
  }

  bool setContext(const GMatrix &ctm)
  {
    context = ctm * localMatrix;
    return context.invert(&invContext);
  }

  void shadeRow(int x, int y, int count, GPixel row[])
  {
    GPoint vec{x + 0.5f, y + 0.5f};
    GPoint local = invContext * vec;

    int width = bitmap.width();
    int height = bitmap.height();
    float x0 = local.x();
    float y0 = local.y();
    float dx = invContext[GMatrix::SX];
    float dy = invContext[GMatrix::KY];
    int x1, y1;

    for (int i = 0; i < count; ++i, x0 += dx, y0 += dy)
    {
      x1 = GFloorToInt(width * tile(x0));
      y1 = GFloorToInt(height * tile(y0));
      row[i] = *bitmap.getAddr(x1, y1);
    }
  }

private:
  const GBitmap bitmap;
  const GMatrix localMatrix;
  GMatrix context;
  GMatrix invContext;

  typedef float (*Tiler)(float);
  Tiler tile;

  static inline float clamp(float a)
  {
    return CLAMP(a, 0.0f, 0.999999f);
  }

  static inline float repeat(float a)
  {
    return a - floorf(a);
  }

  static inline float mirror(float a)
  {
    return 1 - std::abs((a - 2 * floorf(a * 0.5)) - 1);
  }
};

class LGradient : public GShader
{
public:
  LGradient(GPoint newP0, GPoint newP1, const GColor newColors[], int count, GShader::TileMode mode) : p0(newP0), p1(newP1)
  {
    if (count == 0)
      return;
    for (int i = 0; i < count - 1; ++i)
    {
      GColor curr = GColor(newColors[i]);
      colors.push_back(curr);
      colorsDiff.push_back(newColors[i + 1] - curr);
    }
    GColor last = GColor(newColors[count - 1]);
    colors.push_back(last);
    colorsDiff.push_back(last);

    float dx = newP1.x() - newP0.x();
    float dy = newP1.y() - newP0.y();

    localMatrix = GMatrix(dx, -dy, p0.x(), dy, dx, p0.y());

    switch (mode)
    {
    case GShader::TileMode::kRepeat:
      tile = repeat;
      break;
    case GShader::TileMode::kMirror:
      tile = mirror;
      break;
    default:
      tile = clamp;
    }
  }

  bool isOpaque() override
  {
    for (int i = 0; i < colors.size(); ++i)
    {
      if (colors[i].a < 1)
      {
        return false;
      }
    }
    return true;
  }

  bool setContext(const GMatrix &ctm) override
  {
    context = ctm * localMatrix;
    return context.invert(&invContext);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override
  {
    if (colors.size() == 1)
    {
      GPixel base = createPixel(colors[0]);
      for (int i = 0; i < count; ++i)
      {
        row[i] = base;
      }
      return;
    }
    GPoint vec{x + 0.5f, y + 0.5f};
    GPoint local = invContext * vec;
    float x0 = local.x();
    float dx = invContext[GMatrix::SX];
    int numColors = colors.size();
    for (int i = 0; i < count; ++i, x0 += dx)
    {
      float scale = tile(x0) * (numColors - 1);
      int index = GFloorToInt(scale);
      float w = scale - index;
      row[i] = createPixel(colors[index] + w * colorsDiff[index]);
    }
  }

private:
  GPoint p0;
  GPoint p1;
  std::vector<GColor> colors;
  std::vector<GColor> colorsDiff;
  GMatrix context;
  GMatrix invContext;
  GMatrix localMatrix;

  typedef float (*Tiler)(float);
  Tiler tile;

  static inline float clamp(float a)
  {
    return CLAMP(a, 0.0f, 1.0f);
  }

  static inline float repeat(float a)
  {
    return a - floorf(a);
  }

  static inline float mirror(float a)
  {
    return 1 - std::abs((a - 2 * floorf(a * 0.5)) - 1);
  }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode)
{
  return std::unique_ptr<GShader>(new LGradient(p0, p1, colors, count, mode));
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bitmap, const GMatrix &localMatrix, GShader::TileMode mode)
{
  return std::unique_ptr<GShader>(new LShader(bitmap, localMatrix, mode));
};