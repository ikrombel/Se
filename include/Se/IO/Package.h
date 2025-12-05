#pragma once
#include <Se/String.hpp>
#include <Se/IO/FileSystem.h>

namespace Se
{

class Package
{
public:

    virtual std::vector<String> GetEntryNames() const = 0;

    void Scan(std::vector<String>& result, const String& pathName, 
            const String& filter, ScanFlags flags) const;

    void ScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const;


};

}