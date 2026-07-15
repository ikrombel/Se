// Copyright (c) 2017-2020 the rbfx project.

#pragma once

#include <SeResource/BinaryArchive.h>
#include <Se/IO/VectorBuffer.h>

namespace Se
{

/// Base64 output archive.
class Base64OutputArchive : private VectorBuffer, public BinaryOutputArchive
{
public:
    Base64OutputArchive();

    /// Return base64-encoded result.
    String GetBase64() const;
};

/// Base64 input archive.
class Base64InputArchive : private VectorBuffer, public BinaryInputArchive
{
public:
    Base64InputArchive(const String& base64);
};

}
