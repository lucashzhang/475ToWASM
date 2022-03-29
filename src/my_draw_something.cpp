#include "GCanvas.h"
#include "GRect.h"
#include "GColor.h"
#include "GPaint.h"
#include "GMatrix.h"
#include "GBlendMode.h"

std::string GDrawSomething(GCanvas *canvas, GISize dim)
{
    // as fancy as you like

    GPaint paint = GPaint({0, 1, 1, 0.1});
    GPoint verts[] = {{50, 50}, {150, 50}, {175, 150}, {25, 150}};

    
    canvas->drawQuad(verts, nullptr, nullptr, 1, paint);

    return "Kaleidoscope v2";
}