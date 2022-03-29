#include "GCanvas.h"
#include "GPixel.h"
#include "GBitmap.h"
#include "GRect.h"
#include "GPaint.h"
#include "GPath.h"
#include "LPainter.h"
#include "LDot.h"
#include "GShader.h"
#include "LUtil.h"
#include "LTriShader.h"
#include <vector>

class MyCanvas : public GCanvas
{
public:
  MyCanvas(const GBitmap &device) : fDevice(device), screenRect(GIRect::MakeLTRB(0, 0, device.width(), device.height())), rowBuffer(device.width(), 0), columnBuffer(device.height()), ctm(GMatrix()) {}

  void drawPaint(const GPaint &paint) override
  {
    paintRect(screenRect, paint);
  }

  void drawRect(const GRect &rect, const GPaint &paint) override
  {
    const GIRect rounded = rect.round();
    if (ctm == GMatrix())
    {
      paintRect(rounded, paint);
      return;
    }
    GPoint asPoints[4] = {{(float)rounded.left(), (float)rounded.top()},
                          {(float)rounded.right(), (float)rounded.top()},
                          {(float)rounded.right(), (float)rounded.bottom()},
                          {(float)rounded.left(), (float)rounded.bottom()}};

    drawConvexPolygon(asPoints, 4, paint);
  }

  void drawConvexPolygon(const GPoint points[], int count, const GPaint &paint) override
  {
    GPath path;
    path.addPolygon(points, count);
    drawPath(path, paint);
  }

  void drawPath(const GPath &path, const GPaint &paint) override
  {
    const GBlendMode mode = paintToMode(paint);
    if (mode == GBlendMode::kDst)
      return;

    GPath dupPath = GPath(path);
    dupPath.transform(ctm);
    GIRect bounds = dupPath.bounds().round();
    int top = CLAMP(bounds.top(), screenRect.top(), screenRect.bottom());
    int bottom = CLAMP(bounds.bottom(), screenRect.top(), screenRect.bottom());
    LPathToDots(columnBuffer, dupPath, screenRect);
    paintBuffer(top, bottom, paint);
  }

  void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint &paint) override
  {
    const GBlendMode mode = paintToMode(paint);
    if (mode == GBlendMode::kDst || count == 0)
      return;
    GPaint meshPaint = GPaint();
    meshPaint.setBlendMode(mode);
    meshPaint.setRGBA(0, 0, 0, 1);

    bool hasColors = colors != nullptr;
    bool hasTexs = texs != nullptr;
    LTriShader *triShader = nullptr;
    if (hasColors && hasTexs)
      triShader = new LComposeShader(paint.getShader());
    else if (hasColors)
      triShader = new LColorShader();
    else if (hasTexs)
      triShader = new LProxyShader(paint.getShader());
    meshPaint.setShader(triShader);
    int numVerts = 0;
    for (int i = 0; i < count * 3; i++)
    {
      numVerts = std::max(numVerts, indices[i] + 1);
    }

    int vIdx = 0;
    std::vector<GPoint> mappedverts(numVerts);
    ctm.mapPoints(mappedverts.data(), verts, numVerts);
    for (int i = 0; i < count; ++i, vIdx += 3)
    {
      int idx0 = indices[vIdx];
      int idx1 = indices[vIdx + 1];
      int idx2 = indices[vIdx + 2];
      GPoint p0 = verts[idx0];
      GPoint p1 = verts[idx1];
      GPoint p2 = verts[idx2];
      if (hasColors)
      {
        triShader->init(p0, p1, p2, colors[idx0], colors[idx1], colors[idx2]);
      }
      if (hasTexs)
      {
        triShader->init(p0, p1, p2, texs[idx0], texs[idx1], texs[idx2]);
      }
      paintTriangle(mappedverts[idx0], mappedverts[idx1], mappedverts[idx2], meshPaint);
    }

    delete triShader;
  }

  void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint &paint) override
  {
    int numDiv = level + 1;
    int numPts = level + 2;
    float step = 1.0f / numDiv;
    std::vector<GPoint> lerpVerts;
    std::vector<GColor> lerpColors;
    std::vector<GPoint> lerpTexs;
    quadLerp(lerpVerts, verts[0], verts[1], verts[2], verts[3], numPts, step);
    if (colors)
      quadLerp(lerpColors, colors[0], colors[1], colors[2], colors[3], numPts, step);
    if (texs)
      quadLerp(lerpTexs, texs[0], texs[1], texs[2], texs[3], numPts, step);

    int numTri = numDiv * numDiv * 2;
    std::vector<int> indices(numTri * 3);
    int anchor = 0;
    int idx = 0;
    for (int i = 0; i < numDiv; ++i)
    {
      for (int j = 0; j < numDiv; ++j)
      {
        // Triangle ABD
        indices[idx++] = anchor;
        indices[idx++] = anchor + 1;
        indices[idx++] = anchor + numPts;
        // Triangle BDC
        indices[idx++] = anchor + 1;
        indices[idx++] = anchor + numPts;
        indices[idx++] = anchor + numPts + 1;
        ++anchor;
      }
      ++anchor;
    }
    drawMesh(lerpVerts.data(), lerpColors.size() > 0 ? lerpColors.data() : nullptr, lerpTexs.size() > 0 ? lerpTexs.data() : nullptr, numTri, indices.data(), paint);
  }

  void save() override
  {
    saveStates.push_back(GMatrix(ctm));
  }

  void restore() override
  {
    ctm = saveStates.back();
    saveStates.pop_back();
  }

  void concat(const GMatrix &matrix) override
  {
    ctm.preConcat(matrix);
  }

private:
  // Note: we store a copy of the bitmap
  const GBitmap fDevice;
  const GIRect screenRect;
  std::vector<GPixel> rowBuffer;
  std::vector<std::vector<LDot>> columnBuffer;
  std::vector<GMatrix> saveStates;
  GMatrix ctm;

  GIRect clipRects(GIRect rect1, GIRect rect2)
  {
    if (rect1.intersects(rect2))
    {
      int left = std::max(rect1.left(), rect2.left());
      int right = std::min(rect1.right(), rect2.right());
      int top = std::max(rect1.top(), rect2.top());
      int bottom = std::min(rect1.bottom(), rect2.bottom());
      return GIRect::MakeLTRB(left, top, right, bottom);
    }

    return GIRect::MakeLTRB(0, 0, 0, 0);
  }

  void paintRect(const GIRect &rect, const GPaint &paint)
  {
    GIRect clipped = clipRects(rect, screenRect);
    const GBlendMode mode = paintToMode(paint);
    if (mode == GBlendMode::kDst)
      return;

    const GPixel basePixel = createPixel(paint.getColor());
    const Painter painter = modeToPainter(mode);
    GShader *shader = paint.getShader();
    const Filler fill = shader ? shadeRow : fillRow;
    shader && shader->setContext(ctm);

    for (int y = clipped.top(); y < clipped.bottom(); y++)
    {
      fill(clipped.left(), y, rowBuffer.data(), clipped.width(), shader, basePixel);
      paintRow(rowBuffer.data(), fDevice.getAddr(clipped.left(), y), clipped.width(), painter);
    }
  }

  void paintBuffer(int top, int bottom, const GPaint &paint)
  {
    const GBlendMode mode = paintToMode(paint);
    const Painter painter = modeToPainter(mode);
    const GPixel basePixel = createPixel(paint.getColor());
    GShader *shader = paint.getShader();
    const Filler fill = shader ? shadeRow : fillRow;
    if (shader)
    {
      shader->setContext(ctm);
    }

    for (int y = top; y < bottom; y++)
    {
      LDot *currRow = columnBuffer[y].data();
      int numDots = columnBuffer[y].size();
      if (numDots == 0)
      {
        continue;
      }
      std::sort(currRow, currRow + numDots, [](LDot a, LDot b)
                { return a.x < b.x; });
      int w = 0;
      int x0 = 0;

      for (int i = 0; i < numDots; ++i)
      {
        LDot currDot = currRow[i];
        int x = currDot.x;
        if (w == 0)
        {
          x0 = x;
        }
        w += currDot.w;
        if (w == 0 && x0 < x)
        {
          fill(x0, y, rowBuffer.data(), x - x0, shader, basePixel);
          paintRow(rowBuffer.data(), fDevice.getAddr(x0, y), x - x0, painter);
        }
      }
      columnBuffer[y].clear();
    }
  }

  void paintTriangle(const GPoint &p0, const GPoint &p1, const GPoint &p2, const GPaint &paint)
  {
    LEdgeToDots(columnBuffer, p0, p1, screenRect);
    LEdgeToDots(columnBuffer, p1, p2, screenRect);
    LEdgeToDots(columnBuffer, p2, p0, screenRect);
    int top = std::max(GRoundToInt(std::min(p0.y(), std::min(p1.y(), p2.y()))), screenRect.top());
    int bottom = std::min(GRoundToInt(std::max(p0.y(), std::max(p1.y(), p2.y()))), screenRect.bottom());
    paintBuffer(top, bottom, paint);
  }

  template <class T>
  void quadLerp(std::vector<T> &data, const T &a, const T &b, const T &c, const T &d, int num, float step)
  {
    float y = 0;
    for (int i = 0; i < num; ++i, y += step)
    {
      float x = 0;
      for (int j = 0; j < num; ++j, x += step)
      {
        data.push_back(BILERP(x, y, a, b, c, d));
      }
    }
  }
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &device)
{
  return std::unique_ptr<GCanvas>(new MyCanvas(device));
}