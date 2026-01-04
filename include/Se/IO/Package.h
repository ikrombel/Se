#pragma once
#include <Se/String.hpp>
#include <Se/IO/FileSystem.h>

#include <SeArc/ArchiveSerialization.hpp>

namespace Se
{

class Package
{
public:

    virtual ~Package() = default;

    virtual void SerializeInBlock(Se::Archive& archive) {}

    virtual std::vector<String> GetEntryNames() const = 0;

    void Scan(std::vector<String>& result, const String& pathName, 
            const String& filter, ScanFlags flags) const;

    void ScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const;


    //virtual bool IsAutoMount() const { return true; }

protected:

};

}