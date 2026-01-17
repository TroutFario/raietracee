#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Source courtesy of J. Manson
// http://josiahmanson.com/prose/optimize_ppm/

namespace ppmLoader {
using namespace std;
void eat_comment(ifstream& f);

struct RGB {
    unsigned char r, g, b;
};

struct ImageRGB {
    int w, h;
    vector<RGB> data;
};

enum ImageMirrorMode {
    ImageMirror_None,
    ImageMirror_Horizontal,
    ImageMirror_Vertical,
    ImageMirror_Both
};

RGB get_color_simple(const ImageRGB& image, int u, int v, RGB defaultColor, ImageMirrorMode mirrorMode);

RGB get_color_cover(const ImageRGB& image, int u, int v, ImageMirrorMode mirrorMode);

RGB get_color_cover_repeat(const ImageRGB& image, int u, int v, ImageMirrorMode mirrorMode);

RGB get_color_repeat(const ImageRGB& image, int u, int v, ImageMirrorMode mirrorMode);

RGB get_color_clamp(const ImageRGB& image, int u, int v, ImageMirrorMode mirrorMode);

void load_ppm(ImageRGB& img, const string& name);

enum loadedFormat {
    rgb,
    rbg
};

void load_ppm(unsigned char*& pixels, unsigned int& w, unsigned int& h, const string& name, loadedFormat format = rgb);
}  // namespace ppmLoader

#endif
