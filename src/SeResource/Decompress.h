#pragma once

#include <SeResource/Image.h>

namespace Se
{

/// Decompress a DXT compressed image to RGBA.
void DecompressImageDXT(unsigned char* rgba, const void* blocks, 
        int width, int height, int depth, CompressedFormat format);
/// Decompress an ETC1/ETC2 compressed image to RGBA.
void DecompressImageETC(unsigned char* dstImage, const void* blocks, 
        int width, int height, bool hasAlpha);
/// Decompress a PVRTC compressed image to RGBA.
void DecompressImagePVRTC(unsigned char* rgba, const void* blocks, 
        int width, int height, CompressedFormat format);
/// Flip a compressed block vertically.
void FlipBlockVertical(unsigned char* dest, const unsigned char* src, CompressedFormat format);
/// Flip a compressed block horizontally.
void FlipBlockHorizontal(unsigned char* dest, const unsigned char* src, CompressedFormat format);

}
