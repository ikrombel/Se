#include <SeResource/ImageCube.h>

#include <Se/Console.hpp>
#include <Se/IO/FileSystem.h>

#include <SeResource/Image.h>
#include <SeResource/ResourceCache.h>
#include <SeResource/XMLFile.h>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

namespace Se
{

static const std::vector<String> cubeMapLayoutNames = {
    "horizontal",
    "horizontalnvidia",
    "horizontalcross",
    "verticalcross",
    "blender"
};

static std::shared_ptr<Image> GetTileImage(Image* src, int tileX, int tileY, int tileWidth, int tileHeight)
{
    return std::shared_ptr<Image>(
        src->GetSubimage(IntRect(tileX * tileWidth, tileY * tileHeight, (tileX + 1) * tileWidth, (tileY + 1) * tileHeight)));
}

ImageCube::ImageCube() : Resource("ImageCube") {}

ImageCube::~ImageCube() = default;

bool ImageCube::BeginLoad(Deserializer& source)
{
    auto& cache = ResourceCache::Get();

    cache.ResetDependencies(this);

    String texPath, texName, texExt;
    SplitPath(GetName(), texPath, texName, texExt);

    parametersXml_ = std::make_shared<XMLFile>();
    if (!parametersXml_->Load(source))
    {
        parametersXml_.reset();
        return false;
    }

    faceImages_.clear();

    XMLElement textureElem = parametersXml_->GetRoot();
    XMLElement imageElem = textureElem.GetChild("image");
    // Single image and multiple faces with layout
    if (imageElem)
    {
        String name = imageElem.GetAttribute("name");
        // If path is empty, add the XML file path
        if (GetPath(name).empty())
            name = texPath + name;

        std::shared_ptr<Image> image = cache.GetTempResource<Image>(name);
        if (!image)
            return false;

        int faceWidth, faceHeight;
        faceImages_.resize(MAX_CUBEMAP_FACES);

        if (image->IsCubemap())
        {
            faceImages_[FACE_POSITIVE_X] = image;
            faceImages_[FACE_NEGATIVE_X] = faceImages_[FACE_POSITIVE_X]->GetNextSibling();
            faceImages_[FACE_POSITIVE_Y] = faceImages_[FACE_NEGATIVE_X]->GetNextSibling();
            faceImages_[FACE_NEGATIVE_Y] = faceImages_[FACE_POSITIVE_Y]->GetNextSibling();
            faceImages_[FACE_POSITIVE_Z] = faceImages_[FACE_NEGATIVE_Y]->GetNextSibling();
            faceImages_[FACE_NEGATIVE_Z] = faceImages_[FACE_POSITIVE_Z]->GetNextSibling();
        }
        else
        {

            static auto GetStringListIndex = [](const char* value, const std::vector<String>& strings, unsigned defaultIndex) -> unsigned
            {
                unsigned i = 0;
                bool caseSensitive = true;

                while (!strings[i].empty())
                {
                    bool isOk = caseSensitive ? strcmp(value, strings[i].c_str()) : strcasecmp(value, strings[i].c_str());
                    if (!isOk)
                        return i;
                    ++i;
                }

                return defaultIndex;
            };

            CubeMapLayout layout =
                (CubeMapLayout)GetStringListIndex(imageElem.GetAttribute("layout").c_str(), cubeMapLayoutNames, (unsigned)CML_HORIZONTAL);

            switch (layout)
            {
            case CML_HORIZONTAL:
                faceWidth = image->GetWidth() / MAX_CUBEMAP_FACES;
                faceHeight = image->GetHeight();
                faceImages_[FACE_POSITIVE_Z] = GetTileImage(image.get(), 0, 0, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_X] = GetTileImage(image.get(), 1, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Z] = GetTileImage(image.get(), 2, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_X] = GetTileImage(image.get(), 3, 0, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_Y] = GetTileImage(image.get(), 4, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Y] = GetTileImage(image.get(), 5, 0, faceWidth, faceHeight);
                break;

            case CML_HORIZONTALNVIDIA:
                faceWidth = image->GetWidth() / MAX_CUBEMAP_FACES;
                faceHeight = image->GetHeight();
                for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
                    faceImages_[i] = GetTileImage(image.get(), i, 0, faceWidth, faceHeight);
                break;

            case CML_HORIZONTALCROSS:
                faceWidth = image->GetWidth() / 4;
                faceHeight = image->GetHeight() / 3;
                faceImages_[FACE_POSITIVE_Y] = GetTileImage(image.get(), 1, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_X] = GetTileImage(image.get(), 0, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_Z] = GetTileImage(image.get(), 1, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_X] = GetTileImage(image.get(), 2, 1, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Z] = GetTileImage(image.get(), 3, 1, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Y] = GetTileImage(image.get(), 1, 2, faceWidth, faceHeight);
                break;

            case CML_VERTICALCROSS:
                faceWidth = image->GetWidth() / 3;
                faceHeight = image->GetHeight() / 4;
                faceImages_[FACE_POSITIVE_Y] = GetTileImage(image.get(), 1, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_X] = GetTileImage(image.get(), 0, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_Z] = GetTileImage(image.get(), 1, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_X] = GetTileImage(image.get(), 2, 1, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Y] = GetTileImage(image.get(), 1, 2, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Z] = GetTileImage(image.get(), 1, 3, faceWidth, faceHeight);
                if (faceImages_[FACE_NEGATIVE_Z])
                {
                    faceImages_[FACE_NEGATIVE_Z]->FlipVertical();
                    faceImages_[FACE_NEGATIVE_Z]->FlipHorizontal();
                }
                break;

            case CML_BLENDER:
                faceWidth = image->GetWidth() / 3;
                faceHeight = image->GetHeight() / 2;
                faceImages_[FACE_NEGATIVE_X] = GetTileImage(image.get(), 0, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Z] = GetTileImage(image.get(), 1, 0, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_X] = GetTileImage(image.get(), 2, 0, faceWidth, faceHeight);
                faceImages_[FACE_NEGATIVE_Y] = GetTileImage(image.get(), 0, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_Y] = GetTileImage(image.get(), 1, 1, faceWidth, faceHeight);
                faceImages_[FACE_POSITIVE_Z] = GetTileImage(image.get(), 2, 1, faceWidth, faceHeight);
                break;
            }
        }
    }
    // Face per image
    else
    {
        XMLElement faceElem = textureElem.GetChild("face");
        while (faceElem)
        {
            String name = faceElem.GetAttribute("name");

            // If path is empty, add the XML file path
            if (GetPath(name).empty())
                name = texPath + name;

            faceImages_.push_back(cache.GetTempResource<Image>(name));
            cache.StoreResourceDependency(this, name);

            faceElem = faceElem.GetNext("face");
        }
    }

    // Precalculate mip levels if async loading
    if (GetAsyncLoadState() == ASYNC_LOADING)
    {
        for (unsigned i = 0; i < faceImages_.size(); ++i)
        {
            if (faceImages_[i])
                faceImages_[i]->PrecalculateLevels();
        }
    }

    // Calculate width and memory use
    unsigned memoryUse = 0;
    width_ = 0;
    for (unsigned i = 0; i < faceImages_.size(); ++i)
    {
        Image* faceImage = faceImages_[i].get();
        if (faceImage)
        {
            memoryUse += faceImage->GetMemoryUse();

            if (!width_)
                width_ = faceImage->GetWidth();

            if (faceImage->GetWidth() != width_ || faceImage->GetHeight() != width_)
                return false;
        }
    }

    SetMemoryUse(memoryUse);
    return true;
}

std::shared_ptr<ImageCube> ImageCube::GetDecompressedImageLevel(unsigned index) const
{
    auto copy = std::make_shared<ImageCube>();

    copy->parametersXml_ = parametersXml_;
    copy->faceImages_ = faceImages_;
    copy->width_ = std::max<int>(1u, width_ >> index);

    for (std::shared_ptr<Image>& faceImage : copy->faceImages_)
    {
        if (faceImage)
            faceImage = faceImage->GetDecompressedImageLevel(index);
    }

    return copy;
}

std::shared_ptr<ImageCube> ImageCube::GetDecompressedImage() const
{
    return GetDecompressedImageLevel(0);
}

Color ImageCube::SampleNearest(const Vector3& direction) const
{
    const auto projection = ProjectDirectionOnFaceTexel(direction);
    const IntVector2& texel = projection.second;
    return faceImages_[projection.first]->GetPixel(texel.x_, texel.y_);
}

unsigned ImageCube::GetSphericalHarmonicsMipLevel() const
{
    const unsigned maxLevel = LogBaseTwo(width_);
    const unsigned bestLevelCountedFromSmallest = std::min(maxLevel, LogBaseTwo(8)); // 8x8 should be enough for SH
    return maxLevel - bestLevelCountedFromSmallest;
}

#ifdef IMAGECUBE_SPHERICAL_HARMONICS
// worked
SphericalHarmonicsColor9 ImageCube::CalculateSphericalHarmonics() const
{
    SphericalHarmonicsColor9 result;
    float weightSum = 0.0f;

    for (unsigned i = 0; i < 6; ++i)
    {
        const auto face = static_cast<CubeMapFace>(i);

        Image* faceImage = GetImage(face);
        if (!faceImage)
            continue;

        const unsigned bestLevel = GetSphericalHarmonicsMipLevel();
        const unsigned bestLevelWidth = width_ >> bestLevel;
        auto decompressedImage = faceImage->GetDecompressedImageLevel(bestLevel);

        for (int y = 0; y < bestLevelWidth; ++y)
        {
            for (int x = 0; x < bestLevelWidth; ++x)
            {
                const Color sample = decompressedImage->GetPixel(x, y).GammaToLinear();
                const Vector3 offset = ProjectTexelOnCubeLevel(face, x, y, bestLevel);
                const float distance = offset.Length();
                const float weight = 1.0f / (distance * distance * distance);
                const Vector3 direction = offset / distance;

                result += SphericalHarmonicsColor9(direction, sample) * weight;
                weightSum += weight;
            }
        }
    }

    result *= 4.0f * M_PI / weightSum;
    return result;
}
#endif


Vector3 ImageCube::ProjectTexelOnCubeLevel(CubeMapFace face, int x, int y, unsigned level) const
{
    const float u = (x + 0.5f) / (width_ >> level);
    const float v = (y + 0.5f) / (width_ >> level);
    return ProjectUVOnCube(face, { u, v });
}

Vector3 ImageCube::ProjectTexelOnCube(CubeMapFace face, int x, int y) const
{
    return ProjectTexelOnCubeLevel(face, x, y, 0);
}

std::pair<CubeMapFace, IntVector2> ImageCube::ProjectDirectionOnFaceTexel(const Vector3& direction) const
{
    const auto faceUV = ProjectDirectionOnFace(direction);
    const Vector2& uv = faceUV.second;
    const int x = Clamp(FloorToInt(uv.x_ * width_), 0, width_);
    const int y = Clamp(FloorToInt(uv.y_ * width_), 0, width_);
    return { faceUV.first, { x, y } };
}

Vector3 ImageCube::ProjectUVOnCube(CubeMapFace face, const Vector2& uv)
{
    // Convert from [0, 1] to [-1, 1]
    const float u = uv.x_ * 2.0f - 1.0f;
    const float v = uv.y_ * 2.0f - 1.0f;

    switch (face)
    {
    case FACE_POSITIVE_X: return Vector3( 1, -v, -u);
    case FACE_NEGATIVE_X: return Vector3(-1, -v,  u);
    case FACE_POSITIVE_Y: return Vector3( u,  1,  v);
    case FACE_NEGATIVE_Y: return Vector3( u, -1, -v);
    case FACE_POSITIVE_Z: return Vector3( u, -v,  1);
    case FACE_NEGATIVE_Z: return Vector3(-u, -v, -1);
    default: return Vector3::ZERO;
    }
}

std::pair<CubeMapFace, Vector2> ImageCube::ProjectDirectionOnFace(const Vector3& direction)
{
    const float x = direction.x_;
    const float y = direction.y_;
    const float z = direction.z_;

    auto testAxis = [](float axis, float a, float b) { return 2 * axis + M_LARGE_EPSILON > Abs(a) + Abs(b); };

    std::pair<CubeMapFace, Vector2> result;
    if (testAxis(x, y, z))
        result = { FACE_POSITIVE_X, { -z /  x, -y /  x } }; // x =  1, y = -v, z = -u
    else if (testAxis(-x, y, z))
        result = { FACE_NEGATIVE_X, {  z / -x, -y / -x } }; // x = -1, y = -v, z =  u
    else if (testAxis(y, x, z))
        result = { FACE_POSITIVE_Y, {  x /  y,  z /  y } }; // x =  u, y =  1, z =  v
    else if (testAxis(-y, x, z))
        result = { FACE_NEGATIVE_Y, {  x / -y, -z / -y } }; // x =  u, y = -1, z = -v
    else if (testAxis(z, x, y))
        result = { FACE_POSITIVE_Z, {  x /  y, -y /  z } }; // x =  u, y = -v, z =  1
    else // if (testAxis(-z, x, y))
        result = { FACE_NEGATIVE_Z, { -x / -y, -y / -z } }; // x = -u, y = -v, z = -1

    // Convert from [-1, 1] to [0, 1]
    result.second.x_ = result.second.x_ * 0.5f + 0.5f;
    result.second.y_ = result.second.y_ * 0.5f + 0.5f;
    return result;
}

}
