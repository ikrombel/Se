
#pragma once

#include <memory>

#include <Se/Export.hpp>
#include <Se/String.hpp>
//#include "../Core/Variant.h"
#include <Se/StringHash.hpp>
#include <Se/Mutex.hpp>



namespace Se
{

#ifndef StringMap
typedef std::unordered_map<String, String> StringMap;
#endif

class Mutex;
class StringHash;

/// Helper class used for StringHash reversing.
class SE_API StringHashRegister
{
public:
    /// Construct. threadSafe controls whether the RegisterString and GetStringCopy are thread-safe.
    StringHashRegister(bool threadSafe);
    /// Destruct.
    ~StringHashRegister();

    /// Register string for hash reverse mapping. Could be used from StringHash ctor.
    StringHash RegisterString(const StringHash& hash, const String& string);
    /// Register string for hash reverse mapping.
    StringHash RegisterString(const String& string);
    /// Return string for given StringHash. Return empty string if not found.
    String GetStringCopy(const StringHash& hash) const;
    /// Return whether the string in contained in the register.
    bool Contains(const StringHash& hash) const;

    /// Return String for given StringHash. Return value is unsafe to use if RegisterString is called from other threads.
    const String& GetString(const StringHash& hash) const;
    /// Return map of hashes. Return value is unsafe to use if RegisterString is called from other threads.
    const StringMap& GetInternalMap() const { return map_; }

private:
    /// Hash to string map.
    StringMap map_;
    /// Mutex.
    std::unique_ptr<Mutex> mutex_;
};

}
