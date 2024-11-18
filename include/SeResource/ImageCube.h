#pragma once

//#include <GFrost.Graphics/GraphicsDefs.h>
#include <SeResource/Image.h>
#include <SeResource/Resource.h>
//#include <SeMath/SphericalHarmonics.h>

#include <vector>

namespace Se
{

class Deserializer;
class Image;
class XMLFile;

/// Cube map faces.
enum CubeMapFace
{
    FACE_POSITIVE_X = 0,
    FACE_NEGATIVE_X,
    FACE_POSITIVE_Y,
    FACE_NEGATIVE_Y,
    FACE_POSITIVE_Z,
    FACE_NEGATIVE_Z,
    MAX_CUBEMAP_FACES
};

/// Cubemap single image layout modes.
enum CubeMapLayout
{
    CML_HORIZONTAL = 0,
    CML_HORIZONTALNVIDIA,
    CML_HORIZONTALCROSS,
    CML_VERTICALCROSS,
    CML_BLENDER
};

/// Cube texture resource.
class ImageCube : public Resource
{

public:
    /// Construct.
    explicit ImageCube();
    /// Destruct.
    ~ImageCube() override;
    /// Register object factory.
//    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;

    /// Return face images.
    const std::vector<std::shared_ptr<Image>>& GetImages() const { return faceImages_; }
    /// Return parameters XML.
    XMLFile* GetParametersXML() const { return parametersXml_.get(); }
    /// Return image data from a face's zero mip level.
    Image* GetImage(CubeMapFace face) const { return faceImages_[face].get(); }
    /// Return mip level used for SH calculation.
    unsigned GetSphericalHarmonicsMipLevel() const;
    /// Return decompressed cube image mip level.
    std::shared_ptr<ImageCube> GetDecompressedImageLevel(unsigned index) const;
    /// Return decompressed cube image.
    std::shared_ptr<ImageCube> GetDecompressedImage() const;

    /// Return nearest pixel color at given direction.
    Color SampleNearest(const Vector3& direction) const;
    /// Return offset from the center of the unit cube for given texel (assuming zero mip level).
    Vector3 ProjectTexelOnCube(CubeMapFace face, int x, int y) const;
    /// Return offset from the center of the unit cube for given texel.
    Vector3 ProjectTexelOnCubeLevel(CubeMapFace face, int x, int y, unsigned level) const;
    /// Project direction on texel of cubemap face.
    std::pair<CubeMapFace, IntVector2> ProjectDirectionOnFaceTexel(const Vector3& direction) const;
    // /// Calculate spherical harmonics for the cube map.
    // SphericalHarmonicsColor9 CalculateSphericalHarmonics() const;

    /// Project UV onto cube.
    static Vector3 ProjectUVOnCube(CubeMapFace face, const Vector2& uv);
    /// Project direction onto cubemap.
    static std::pair<CubeMapFace, Vector2> ProjectDirectionOnFace(const Vector3& direction);

private:
    /// Face images.
    std::vector<std::shared_ptr<Image>> faceImages_;
    /// Parameter file.
    std::shared_ptr<XMLFile> parametersXml_;
    /// Cube width.
    int width_{};
};

}
