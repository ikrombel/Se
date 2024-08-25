#pragma once

#include <SeMath/Color.hpp>
#include <SeMath/Rect.hpp>
#include <SeResource/Resource.h>

#include <memory>

namespace Se
{

static const int COLOR_LUT_SIZE = 16;

/// Supported compressed image formats.
enum CompressedFormat
{
    CF_NONE = 0,
    CF_RGBA,
    CF_DXT1,
    CF_DXT3,
    CF_DXT5,
    CF_ETC1,
    CF_ETC2_RGB,
    CF_ETC2_RGBA,
    CF_PVRTC_RGB_2BPP,
    CF_PVRTC_RGBA_2BPP,
    CF_PVRTC_RGB_4BPP,
    CF_PVRTC_RGBA_4BPP,
};

/// Compressed image mip level.
struct GFROST_API CompressedLevel
{
    /// Decompress to RGBA. The destination buffer required is width * height * 4 bytes. Return true if successful.
    bool Decompress(unsigned char* dest) const;

    /// Compressed image data.
    unsigned char* data_{};
    /// Compression format.
    CompressedFormat format_{CF_NONE};
    /// Width.
    int width_{};
    /// Height.
    int height_{};
    /// Depth.
    int depth_{};
    /// Block size in bytes.
    unsigned blockSize_{};
    /// Total data size in bytes.
    unsigned dataSize_{};
    /// Row size in bytes.
    unsigned rowSize_{};
    /// Number of rows.
    unsigned rows_{};
};

/// %Image resource.
class GFROST_API Image : public Resource
{

public:
    /// Construct empty.
    explicit Image();
    /// Destruct.
    ~Image(); // override;

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Save the image to a stream. Regardless of original format, the image is saved as png. Compressed image data is not supported. Return true if successful.
    bool Save(Serializer& dest) const override;
    /// Save the image to a file. Format of the image is determined by file extension. JPG is saved with maximum quality.
    bool SaveFile(const FileIdentifier& fileName) const override;

    /// Set 2D size and number of color components. Old image data will be destroyed and new data is undefined. Return true if successful.
    bool SetSize(int width, int height, unsigned components);
    /// Set 3D size and number of color components. Old image data will be destroyed and new data is undefined. Return true if successful.
    bool SetSize(int width, int height, int depth, unsigned components);
    /// Set new image data.
    void SetData(const unsigned char* pixelData);
    /// Set a 2D pixel.
    void SetPixel(int x, int y, const Color& color);
    /// Set a 3D pixel.
    void SetPixel(int x, int y, int z, const Color& color);
    /// Set a 2D pixel with an integer color. R component is in the 8 lowest bits.
    void SetPixelInt(int x, int y, unsigned uintColor);
    /// Set a 3D pixel with an integer color. R component is in the 8 lowest bits.
    void SetPixelInt(int x, int y, int z, unsigned uintColor);
    /// Load as color LUT. Return true if successful.
    bool LoadColorLUT(Deserializer& source);
    /// Flip image horizontally. Return true if successful.
    bool FlipHorizontal();
    /// Flip image vertically. Return true if successful.
    bool FlipVertical();
    /// Resize image by bilinear resampling. Return true if successful.
    bool Resize(int width, int height);
    /// Clear the image with a color.
    void Clear(const Color& color);
    /// Clear the image with an integer color. R component is in the 8 lowest bits.
    void ClearInt(unsigned uintColor);
    /// Save in BMP format. Return true if successful.
    bool SaveBMP(const String& fileName) const;
    /// Save in PNG format. Return true if successful.
    bool SavePNG(const String& fileName) const;
    /// Save in TGA format. Return true if successful.
    bool SaveTGA(const String& fileName) const;
    /// Save in JPG format with specified quality. Return true if successful.
    bool SaveJPG(const String& fileName, int quality) const;
    /// Save in DDS format. Only uncompressed RGBA images are supported. Return true if successful.
    bool SaveDDS(const String& fileName) const;
    /// Save in WebP format with minimum (fastest) or specified compression. Return true if successful. Fails always if WebP support is not compiled in.
    bool SaveWEBP(const String& fileName, float compression = 0.0f) const;
    /// Whether this texture is detected as a cubemap, only relevant for DDS.
    bool IsCubemap() const { return cubemap_; }
    /// Whether this texture has been detected as a volume, only relevant for DDS.
    bool IsArray() const { return array_; }
    /// Whether this texture is in sRGB, only relevant for DDS.
    bool IsSRGB() const { return sRGB_; }

    /// Return a 2D pixel color.
    Color GetPixel(int x, int y) const;
    /// Return a 3D pixel color.
    Color GetPixel(int x, int y, int z) const;
    /// Return a 2D pixel integer color. R component is in the 8 lowest bits.
    unsigned GetPixelInt(int x, int y) const;
    /// Return a 3D pixel integer color. R component is in the 8 lowest bits.
    unsigned GetPixelInt(int x, int y, int z) const;
    /// Return a bilinearly sampled 2D pixel color. X and Y have the range 0-1.
    Color GetPixelBilinear(float x, float y) const;
    /// Return a trilinearly sampled 3D pixel color. X, Y and Z have the range 0-1.
    Color GetPixelTrilinear(float x, float y, float z) const;

    /// Return width.
    int GetWidth() const { return width_; }

    /// Return height.
    int GetHeight() const { return height_; }

    /// Return depth.
    int GetDepth() const { return depth_; }

    /// Return size of the image.
    IntVector3 GetSize() const { return { width_, height_, depth_ }; }

    /// Return number of color components.
    unsigned GetComponents() const { return components_; }

    /// Return pixel data.
    unsigned char* GetData() const { return data_.get(); }

    /// Return whether is compressed.
    bool IsCompressed() const { return compressedFormat_ != CF_NONE; }

    /// Return compressed format.
    CompressedFormat GetCompressedFormat() const { return compressedFormat_; }

    /// Return number of compressed mip levels. Returns 0 if the image is has not been loaded from a source file containing multiple mip levels.
    unsigned GetNumCompressedLevels() const { return numCompressedLevels_; }

    /// Return next mip level by bilinear filtering. Note that if the image is already 1x1x1, will keep returning an image of that size.
    std::shared_ptr<Image> GetNextLevel() const;
    /// Return the next sibling image of an array or cubemap.
    std::shared_ptr<Image> GetNextSibling() const { return nextSibling_;  }
    /// Return image converted to 4-component (RGBA) to circumvent modern rendering API's not supporting e.g. the luminance-alpha format.
    std::shared_ptr<Image> ConvertToRGBA() const;
    /// Return a compressed mip level.
    CompressedLevel GetCompressedLevel(unsigned index) const;
    ///
    std::shared_ptr<Image> GetDecompressedImage() const;
    /// Return LOD of decompressed image in RGBA format.
    std::shared_ptr<Image> GetDecompressedImageLevel(unsigned index) const;
    /// Return subimage from the image by the defined rect or null if failed. 3D images are not supported. You must free the subimage yourself.
    Image* GetSubimage(const IntRect& rect) const;
    /// Precalculate the mip levels. Used by asynchronous texture loading.
    void PrecalculateLevels();
    /// Whether this texture has an alpha channel
    bool HasAlphaChannel() const;
    /// Copy contents of the image into the defined rect, scaling if necessary. This image should already be large enough to include the rect. Compressed and 3D images are not supported.
    bool SetSubimage(const Image* image, const IntRect& rect);
    /// Clean up the mip levels.
    void CleanupLevels();
    /// Get all stored mip levels starting from this.
    void GetLevels(std::vector<Image*>& levels);
    /// Get all stored mip levels starting from this.
    void GetLevels(std::vector<const Image*>& levels) const;
    ///
    int GetBits() { return bits_; }

    bool IsHDR() {return isHDR_; }

private:
    /// Decode an image using stb_image.
    static unsigned char* GetImageData(Deserializer& source, int& width, int& height, int& bits, unsigned& components, bool& isHDR);
    /// Free an image file's pixel data.
    static void FreeImageData(unsigned char* pixelData);

    /// Width.
    int width_{};
    /// Height.
    int height_{};
    /// Depth.
    int depth_{};
    /// Number of color components.
    unsigned components_{};
    /// Number of compressed mip levels.
    unsigned numCompressedLevels_{};
    /// Cubemap status if DDS.
    bool cubemap_{};
    /// Texture array status if DDS.
    bool array_{};
    /// Data is sRGB.
    bool sRGB_{};
    /// Compressed format.
    CompressedFormat compressedFormat_{CF_NONE};
    /// Pixel data.
    std::shared_ptr<unsigned char> data_;
    /// Precalculated mip level image.
    std::shared_ptr<Image> nextLevel_;
    /// Next texture array or cube map image.
    std::shared_ptr<Image> nextSibling_;

    int sizeOfChannel_{0};

    int bits_{32};

    bool isHDR_{false};
};

struct ImageLoadDesc
{
    const char* filePath;
    uint32_t outWidth;
    uint32_t outHeight;
    uint32_t outBits;
    bool isHDR;
    bool flipY         = false;
    bool srgb          = true;
    uint32_t maxWidth  = 2048;
    uint32_t maxHeight = 2048;
    uint8_t* outPixels;
};


#ifndef __ANDROID__
GFROST_API uint8_t* LoadImageFromFile(const char* filename, uint32_t* width = nullptr, uint32_t* height = nullptr, uint32_t* bits = nullptr, bool* isHDR = nullptr, bool flipY = false, bool srgb = true);
GFROST_API uint8_t* LoadImageFromFile(const String& filename, uint32_t* width = nullptr, uint32_t* height = nullptr, uint32_t* bits = nullptr, bool* isHDR = nullptr, bool flipY = false, bool srgb = true);

GFROST_API bool LoadImageFromFile(ImageLoadDesc& desc);

GFROST_API void SetMaxImageDimensions(uint32_t width, uint32_t height);

GFROST_API void GetMaxImageDimensions(uint32_t& width, uint32_t& height);

#endif

}