#include <SeResource/ImageSVG.h>

#include <Se/Console.hpp>
#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"

#include <vector>
#include <string>
#include <memory>

namespace Se
{

bool ImageSVG::BeginLoad(Deserializer& source)
{
    String fileData = source.ReadString();

    NSVGimage *nsvgImage = nsvgParse(fileData.data(), "px", dpi_);
    if (nsvgImage == nullptr) {
      SE_LOG_ERROR("Error loading SVG file: {}", GetName());
      return false;
    }

    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (rast == nullptr) {
      SE_LOG_ERROR("Error creating SVG rasterizer");
      nsvgDelete(nsvgImage);
      return false;
    }

    if (this->GetWidth() == 0 || this->GetHeight() == 0) {
      //this->SetSize(256, 256, 4);
      this->SetSize(nsvgImage->width, nsvgImage->height, 4);
    }

    int width = this->GetWidth();  // Increased size for better quality
    int height = this->GetHeight(); // Increased size for better quality


//    unsigned char *pixels = new unsigned char[width * height * 4];
    nsvgRasterize(rast, nsvgImage, 0, 0, width / nsvgImage->width, this->GetData(), width,
                  height, width * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(nsvgImage);
    return true;
}

}