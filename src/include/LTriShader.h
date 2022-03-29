#include "GShader.h"
#include "GColor.h"
#include "GPoint.h"
#include "GMatrix.h"
#include "LUtil.h"
#include "LPainter.h"

class LTriShader : public GShader
{
public:
  virtual void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GColor &c0, const GColor &c1, const GColor &c2) {}
  virtual void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPoint &t0, const GPoint &t1, const GPoint &t2) {}
};

class LColorShader : public LTriShader
{
public:
  LColorShader() {}

  void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GColor &c0, const GColor &c1, const GColor &c2) override
  {
    _c0 = c0;
    _c1 = c1;
    _c2 = c2;
    m[GMatrix::SX] = p1.x() - p0.x();
    m[GMatrix::KX] = p2.x() - p0.x();
    m[GMatrix::TX] = p0.x();
    m[GMatrix::KY] = p1.y() - p0.y();
    m[GMatrix::SY] = p2.y() - p0.y();
    m[GMatrix::TY] = p0.y();
  }

  bool isOpaque() override
  {
    return _c0.a == 1.0f && _c1.a == 1.0f && _c2.a == 1.0f;
  }

  bool setContext(const GMatrix &ctm) override
  {
    GMatrix context = ctm * m;
    return context.invert(&invContext);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override
  {
    GPoint vec{x + 0.5f, y + 0.5f};
    GPoint local = invContext * vec;
    GColor c = local.x() * (_c1 - _c0) + local.y() * (_c2 - _c0) + _c0;
    GColor dc = invContext[GMatrix::SX] * (_c1 - _c0) + invContext[GMatrix::KY] * (_c2 - _c0);
    for (int i = 0; i < count; ++i, c += dc)
    {
      row[i] = createPixel(c);
    }
  }

private:
  GColor _c0;
  GColor _c1;
  GColor _c2;
  GMatrix invContext;
  GMatrix m;
};

class LProxyShader : public LTriShader
{
public:
  LProxyShader(GShader *shader) : real(shader) {}

  void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPoint &t0, const GPoint &t1, const GPoint &t2) override
  {
    GPoint tD1 = t1 - t0;
    GPoint tD2 = t2 - t0;
    GMatrix t(tD1.x(), tD2.x(), t0.x(), tD1.y(), tD2.y(), t0.y());
    GPoint pD1 = p1 - p0;
    GPoint pD2 = p2 - p0;
    GMatrix p(pD1.x(), pD2.x(), p0.x(), pD1.y(), pD2.y(), p0.y());
    t.invert(&t);
    m = p * t;
  }

  bool isOpaque() override
  {
    return real->isOpaque();
  }

  bool setContext(const GMatrix &ctm) override
  {
    return real->setContext(ctm * m);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override
  {
    real->shadeRow(x, y, count, row);
  }

private:
  GShader *real;
  GMatrix m;
};

class LComposeShader : public LTriShader
{
public:
  LComposeShader(LTriShader *first, LTriShader *second) : shader0(first), shader1(second) {}
  LComposeShader(GShader *shader)
  {
    shader0 = new LColorShader();
    shader1 = new LProxyShader(shader);
  }

  ~LComposeShader()
  {
    delete shader0;
    delete shader1;
  }

  void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GColor &c0, const GColor &c1, const GColor &c2) override
  {
    shader0->init(p0, p1, p2, c0, c1, c2);
  }

  void init(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPoint &t0, const GPoint &t1, const GPoint &t2) override
  {
    shader1->init(p0, p1, p2, t0, t1, t2);
  }

  bool isOpaque() override
  {
    return shader0->isOpaque() && shader1->isOpaque();
  }

  bool setContext(const GMatrix &ctm) override
  {
    return shader0->setContext(ctm) && shader1->setContext(ctm);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override
  {
    std::vector<GPixel> temp(count);
    shader0->shadeRow(x, y, count, temp.data());
    shader1->shadeRow(x, y, count, row);
    paintRow(temp.data(), row, count, kMult);
  }

private:
  LTriShader *shader0;
  LTriShader *shader1;
};