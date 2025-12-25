#pragma once

#include <SeResource/Image.h>

namespace Se
{

class Deserializer;


/// SVG Image resource.
class ImageSVG : public Image
{
public:
    ImageSVG() = default;
    ~ImageSVG() = default;

    inline static String GetTypeStatic() { return "ImageSVG"; }
   
    bool BeginLoad(Deserializer& source) override;

    void SetDPI(float dpi) { dpi_ = dpi; }

    float GetDPI() const { return dpi_; }

private:
    float dpi_{96.0f};

};

}