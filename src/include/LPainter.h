#ifndef LPAINTERDEF
#define LPAINTERDEF

#include "GMath.h"
#include "GPixel.h"
#include "GColor.h"
#include "GBlendMode.h"
#include "GPaint.h"
#include "GShader.h"

#define FLOATTOPXLINT(val) (GRoundToInt(val * 255))
#define DIV255(val) ((val + 128) * 257 >> 16)

typedef GPixel (*Painter)(GPixel, GPixel);

static inline GPixel createPixel(GColor color)
{
    color = color.pinToUnit();
    uint8_t a = FLOATTOPXLINT(color.a);
    uint8_t r = FLOATTOPXLINT(color.r * color.a);
    uint8_t g = FLOATTOPXLINT(color.g * color.a);
    uint8_t b = FLOATTOPXLINT(color.b * color.a);
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel scale255(GPixel pxl, uint8_t scale)
{
    switch (scale)
    {
    case 255:
        return pxl;
    case 0:
        return 0;
    default:
        uint8_t a = DIV255(GPixel_GetA(pxl) * scale);
        uint8_t r = DIV255(GPixel_GetR(pxl) * scale);
        uint8_t g = DIV255(GPixel_GetG(pxl) * scale);
        uint8_t b = DIV255(GPixel_GetB(pxl) * scale);
        return GPixel_PackARGB(a, r, g, b);
    }
}

static inline GPixel kClear(GPixel src, GPixel dst)
{
    return 0;
}
static inline GPixel kSrc(GPixel src, GPixel dst)
{
    return src;
}

static inline GPixel kDst(GPixel src, GPixel dst)
{
    return dst;
}

static inline GPixel kSrcOver(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    return src + scale255(dst, 255 - srcAlpha);
}

static inline GPixel kDstOver(GPixel src, GPixel dst)
{
    uint8_t dstAlpha = GPixel_GetA(dst);
    return dst + scale255(src, 255 - dstAlpha);
}

static inline GPixel kSrcIn(GPixel src, GPixel dst)
{
    uint8_t dstAlpha = GPixel_GetA(dst);
    return scale255(src, dstAlpha);
}

static inline GPixel kDstIn(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    return scale255(dst, srcAlpha);
}

static inline GPixel kSrcOut(GPixel src, GPixel dst)
{
    uint8_t dstAlpha = GPixel_GetA(dst);
    return scale255(src, 255 - dstAlpha);
}

static inline GPixel kDstOut(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    return scale255(dst, 255 - srcAlpha);
}

static inline GPixel kSrcATop(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    uint8_t dstAlpha = GPixel_GetA(dst);
    uint8_t a = DIV255(dstAlpha * GPixel_GetA(src) + (255 - srcAlpha) * GPixel_GetA(dst));
    uint8_t r = DIV255(dstAlpha * GPixel_GetR(src) + (255 - srcAlpha) * GPixel_GetR(dst));
    uint8_t g = DIV255(dstAlpha * GPixel_GetG(src) + (255 - srcAlpha) * GPixel_GetG(dst));
    uint8_t b = DIV255(dstAlpha * GPixel_GetB(src) + (255 - srcAlpha) * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel kDstATop(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    uint8_t dstAlpha = GPixel_GetA(dst);
    uint8_t a = DIV255(srcAlpha * GPixel_GetA(dst) + (255 - dstAlpha) * GPixel_GetA(src));
    uint8_t r = DIV255(srcAlpha * GPixel_GetR(dst) + (255 - dstAlpha) * GPixel_GetR(src));
    uint8_t g = DIV255(srcAlpha * GPixel_GetG(dst) + (255 - dstAlpha) * GPixel_GetG(src));
    uint8_t b = DIV255(srcAlpha * GPixel_GetB(dst) + (255 - dstAlpha) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel kXor(GPixel src, GPixel dst)
{
    uint8_t srcAlpha = GPixel_GetA(src);
    uint8_t dstAlpha = GPixel_GetA(dst);
    uint8_t a = DIV255((255 - srcAlpha) * GPixel_GetA(dst) + (255 - dstAlpha) * GPixel_GetA(src));
    uint8_t r = DIV255((255 - srcAlpha) * GPixel_GetR(dst) + (255 - dstAlpha) * GPixel_GetR(src));
    uint8_t g = DIV255((255 - srcAlpha) * GPixel_GetG(dst) + (255 - dstAlpha) * GPixel_GetG(src));
    uint8_t b = DIV255((255 - srcAlpha) * GPixel_GetB(dst) + (255 - dstAlpha) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel kMult(GPixel src, GPixel dst)
{
    uint8_t a = DIV255(GPixel_GetA(src) * GPixel_GetA(dst));
    uint8_t r = DIV255(GPixel_GetR(src) * GPixel_GetR(dst));
    uint8_t g = DIV255(GPixel_GetG(src) * GPixel_GetG(dst));
    uint8_t b = DIV255(GPixel_GetB(src) * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}

static inline GBlendMode translate0(GBlendMode mode)
{
    switch ((int)mode)
    {
    case 0:
        return GBlendMode::kClear;
    case 1:
        return GBlendMode::kClear;
    case 2:
        return GBlendMode::kDst;
    case 3:
        return GBlendMode::kDst;
    case 4:
        return GBlendMode::kDst;
    case 5:
        return GBlendMode::kClear;
    case 6:
        return GBlendMode::kClear;
    case 7:
        return GBlendMode::kClear;
    case 8:
        return GBlendMode::kDst;
    case 9:
        return GBlendMode::kDst;
    case 10:
        return GBlendMode::kClear;
    case 11:
        return GBlendMode::kDst;
    default:
        return GBlendMode::kClear;
    }
}

static inline GBlendMode translate255(GBlendMode mode)
{
    switch ((int)mode)
    {
    case 0:
        return GBlendMode::kClear;
    case 1:
        return GBlendMode::kSrc;
    case 2:
        return GBlendMode::kDst;
    case 3:
        return GBlendMode::kSrc;
    case 4:
        return GBlendMode::kDstOver;
    case 5:
        return GBlendMode::kSrcIn;
    case 6:
        return GBlendMode::kDst;
    case 7:
        return GBlendMode::kSrcOut;
    case 8:
        return GBlendMode::kDstOut;
    case 9:
        return GBlendMode::kSrcIn;
    case 10:
        return GBlendMode::kDstOver;
    case 11:
        return GBlendMode::kSrcOut;
    default:
        return GBlendMode::kClear;
    }
}

static inline GBlendMode paintToMode(const GPaint paint)
{
    GShader *shader = paint.getShader();
    GBlendMode mode = paint.getBlendMode();

    if (shader)
    {
        return shader->isOpaque() ? translate255(mode) : mode;
    }

    uint8_t alpha = FLOATTOPXLINT(paint.getAlpha());

    switch (alpha)
    {
    case 0:
        return translate0(mode);
    case 255:
        return translate255(mode);
    default:
        return mode;
    }
}

static inline Painter modeToPainter(const GBlendMode mode)
{
    switch ((int)mode)
    {
    case 0:
        return kClear;
    case 1:
        return kSrc;
    case 2:
        return kDst;
    case 3:
        return kSrcOver;
    case 4:
        return kDstOver;
    case 5:
        return kSrcIn;
    case 6:
        return kDstIn;
    case 7:
        return kSrcOut;
    case 8:
        return kDstOut;
    case 9:
        return kSrcATop;
    case 10:
        return kDstATop;
    case 11:
        return kXor;
    default:
        return kClear;
    }
}

static inline void paintRow(GPixel src[], GPixel dst[], int length, Painter painter)
{
    // This points to the beginning of the row
    for (int i = 0; i < length; ++i)
    {
        // This fills the rest of the row using pointer arithmetic
        dst[i] = painter(src[i], dst[i]);
    }
}

typedef void (*Filler)(int, int, GPixel[], int, GShader *, GPixel);

static inline void shadeRow(int x, int y, GPixel dst[], int length, GShader *shader, GPixel base)
{
    shader->shadeRow(x, y, length, dst);
}

static inline void fillRow(int x, int y, GPixel dst[], int length, GShader *shader, GPixel base)
{
    for (int i = 0; i < length; ++i)
    {
        dst[i] = base;
    }
}

#endif