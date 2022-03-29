#include <emscripten/val.h>
#include <string>
#include <iostream>
#include <vector>
#include "GBitmap.h"
#include "GCanvas.h"

#define WIDTH 256
#define HEIGHT 256

using emscripten::val;

// Use thread_local when you want to retrieve & cache a global JS variable once per thread.
thread_local const val document = val::global("document");

// …

void convertToBuffer(const GPixel src[], int width, uint8_t dst[])
{
  for (int i = 0; i < width; i++)
  {
    GPixel c = *src++;
    int a = GPixel_GetA(c);
    int r = GPixel_GetR(c);
    int g = GPixel_GetG(c);
    int b = GPixel_GetB(c);

    // PNG requires unpremultiplied, but GPixel is premultiplied
    if (0 != a && 255 != a)
    {
      r = (r * 255 + a / 2) / a;
      g = (g * 255 + a / 2) / a;
      b = (b * 255 + a / 2) / a;
    }
    *dst++ = r;
    *dst++ = g;
    *dst++ = b;
    *dst++ = a;
  }
}

int main()
{
  val canvas = document.call<val>("getElementById", val("canvas"));

  canvas.set("width", WIDTH);
  canvas.set("height", HEIGHT);

  GBitmap bitmap;
  bitmap.alloc(WIDTH, HEIGHT);
  auto cv = GCreateCanvas(bitmap);
  std::string title = GDrawSomething(cv.get(), {256, 256});

  val ctx = canvas.call<val>("getContext", val("2d"));

  const GPixel *src = bitmap.pixels();
  auto pixels = std::vector<uint8_t>(bitmap.height() * bitmap.width() * 4);
  auto dst = pixels.data();

  for (int y = 0; y < HEIGHT; ++y)
  {
    convertToBuffer(src, WIDTH, dst);
    src += bitmap.rowBytes() / 4;
    dst += WIDTH * 4;
  }
  

  std::cout << title << std::endl;

  return 0;
}