#pragma once

#include <SeReflection/Reflected.hpp>

namespace Se {

class RefObject
{
    RefObject(ReflectedObject* obj) : obj_(obj)
    {

    }

    


private:
    ReflectedObject* obj_;

};

inline void SerializeObject(ReflectedObject* obj, const String& name = String::EMPTY)
{

    // auto attributeNames = obj->GetAttriburesNames();

    // for (auto& attrName : attributeNames)
    // {
    //     auto attr = obj->FindAttribute(attrName);

    // }

}


} // namespace Se