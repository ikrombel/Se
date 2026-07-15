// Copyright (c) 2008-2022 the Urho3D project.

//#include "../Precompiled.h"

#include "StringHashRegister.h"
#include "Mutex.hpp"
#include "Console.hpp"
#include "String.hpp"

#include <cstdio>

// #include "../DebugNew.h"

namespace Se
{

StringHashRegister::StringHashRegister(bool threadSafe)
{
    if (threadSafe)
        mutex_ = std::make_unique<Mutex>();
}


StringHashRegister::~StringHashRegister()       // NOLINT(hicpp-use-equals-default, modernize-use-equals-default)
{
    // Keep destructor here to let mutex_ destruct
}

StringHash StringHashRegister::RegisterString(const StringHash& hash, const String& string)
{
    if (mutex_)
        mutex_->Acquire();

    auto iter = map_.find(hash);
    if (iter == map_.end())
    {
        map_.emplace(hash, String(string));
    }
    else if (String(iter->second) != string)
    {
        SE_LOG_WARNING("StringHash collision detected! Both \"{}\" and \"{}\" have hash #{}",
            string, iter->second.c_str(), hash.ToString().c_str());
    }

    if (mutex_)
        mutex_->Release();

    return hash;
}

StringHash StringHashRegister::RegisterString(const String& string)
{
    StringHash hash(string);
    return RegisterString(hash, string);
}

String StringHashRegister::GetStringCopy(const StringHash& hash) const
{
    if (mutex_)
        mutex_->Acquire();

    const String copy = GetString(hash);

    if (mutex_)
        mutex_->Release();

    return copy;
}

bool StringHashRegister::Contains(const StringHash& hash) const
{
    if (mutex_)
        mutex_->Acquire();

    const bool contains = map_.find(hash) != map_.end();

    if (mutex_)
        mutex_->Release();

    return contains;
}

const String& StringHashRegister::GetString(const StringHash& hash) const
{
    auto iter = map_.find(hash);
    return iter == map_.end() ? String::EMPTY : iter->second;
}

}
