#include "GMatrix.h"
#include "math.h"

GMatrix::GMatrix() : GMatrix(1, 0, 0, 0, 1, 0) {}

GMatrix GMatrix::Translate(float tx, float ty)
{
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy)
{
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians)
{
    float cosTheta = std::cos(radians);
    float sinTheta = std::sin(radians);

    return GMatrix(cosTheta, -sinTheta, 0, sinTheta, cosTheta, 0);
}

GMatrix GMatrix::Concat(const GMatrix &a, const GMatrix &b)
{
    return GMatrix(
        a[SX] * b[SX] + a[KX] * b[KY],
        a[SX] * b[KX] + a[KX] * b[SY],
        a[SX] * b[TX] + a[KX] * b[TY] + a[TX],
        a[KY] * b[SX] + a[SY] * b[KY],
        a[KY] * b[KX] + a[SY] * b[SY],
        a[KY] * b[TX] + a[SY] * b[TY] + a[TY]);
}

bool GMatrix::invert(GMatrix *inverse) const
{
    float det = fMat[SX] * fMat[SY] - fMat[KX] * fMat[KY];
    if (det == 0)
    {
        return false;
    }
    float invDet = 1 / det;
    float a = fMat[SX];
    float b = fMat[KX];
    float c = fMat[TX];
    float d = fMat[KY];
    float e = fMat[SY];
    float f = fMat[TY];

    *inverse = GMatrix(
        e * invDet,
        -b * invDet,
        -(c * e - b * f) * invDet,
        -d * invDet,
        a * invDet,
        (c * d - a * f) * invDet);

    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const
{
    float a = fMat[SX];
    float b = fMat[KX];
    float c = fMat[TX];
    float d = fMat[KY];
    float e = fMat[SY];
    float f = fMat[TY];
    for (int i = 0; i < count; ++i)
    {
        float x = src[i].x();
        float y = src[i].y();
        dst[i] = {a * x + b * y + c, d * x + e * y + f};
    }
}