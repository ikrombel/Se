#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <Se/Console.hpp>
#include <Se/IO/MemoryBuffer.hpp>

#include <SeResource/JSONArchive.h>

#include <SeResource/JSONFile.h>
//#include "JSONArchive.h"


namespace Se
{

using namespace rapidjson;


// Convert rapidjson value to JSON value.
static void ToJSONValue(JSONValue& jsonValue, const rapidjson::Value& rapidjsonValue)
{
    switch (rapidjsonValue.GetType())
    {
    case kNullType:
        // Reset to null type
        jsonValue.SetType(VALUE_NULL);
        break;

    case kFalseType:
        jsonValue = false;
        break;

    case kTrueType:
        jsonValue = true;
        break;

    case kNumberType:
        if (rapidjsonValue.IsInt())
            jsonValue = rapidjsonValue.GetInt();
        else if (rapidjsonValue.IsUint())
            jsonValue = rapidjsonValue.GetUint();
        else
            jsonValue = rapidjsonValue.GetDouble();
        break;

    case kStringType:
        jsonValue = rapidjsonValue.GetString();
        break;

    case kArrayType:
        {
            jsonValue.Resize(rapidjsonValue.Size());
            for (unsigned i = 0; i < rapidjsonValue.Size(); ++i)
            {
                ToJSONValue(jsonValue[i], rapidjsonValue[i]);
            }
        }
        break;

    case kObjectType:
        {
            jsonValue.SetType(VALUE_OBJECT);
            for (rapidjson::Value::ConstMemberIterator i = rapidjsonValue.MemberBegin(); i != rapidjsonValue.MemberEnd(); ++i)
            {
                JSONValue& value = jsonValue[String(i->name.GetString())];
                ToJSONValue(value, i->value);
            }
        }
        break;

    default:
        break;
    }
}

bool JSONFile::BeginLoad(Deserializer& source)
{
    unsigned dataSize = source.GetSize();
    if (!dataSize && !source.GetName().empty())
    {
        SE_LOG_ERROR("Zero sized JSON data in " + source.GetName());
        return false;
    }

    std::shared_ptr<char> buffer(new char[dataSize + 1], std::default_delete<char[]>());
    if (source.Read(buffer.get(), dataSize) != dataSize)
        return false;
    buffer.get()[dataSize] = '\0';

    rapidjson::Document document;
    if (document.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(buffer.get()).HasParseError())
    {
        SE_LOG_ERROR("Could not parse JSON data from " + source.GetName());
        return false;
    }

    ToJSONValue(root_, document);

    //SetMemoryUse(dataSize);

    return true;
}

static void ToRapidjsonValue(rapidjson::Value& rapidjsonValue, const JSONValue& jsonValue, rapidjson::MemoryPoolAllocator<>& allocator)
{
    switch (jsonValue.GetValueType())
    {
    case VALUE_NULL:
        rapidjsonValue.SetNull();
        break;

    case VALUE_BOOL:
        rapidjsonValue.SetBool(jsonValue.GetBool());
        break;

    case VALUE_NUMBER:
        {
            switch (jsonValue.GetNumberType())
            {
            case VALUENT_INT:
                rapidjsonValue.SetInt(jsonValue.GetInt());
                break;

            case VALUENT_UINT:
                rapidjsonValue.SetUint(jsonValue.GetUInt());
                break;

            default:
                rapidjsonValue.SetDouble(jsonValue.GetDouble());
                break;
            }
        }
        break;

    case VALUE_STRING:
        rapidjsonValue.SetString(jsonValue.GetCString(), allocator);
        break;

    case VALUE_ARRAY:
        {
            const JSONArray& jsonArray = jsonValue.GetArray();

            rapidjsonValue.SetArray();
            rapidjsonValue.Reserve(jsonArray.size(), allocator);

            for (unsigned i = 0; i < jsonArray.size(); ++i)
            {
                rapidjson::Value value;
                ToRapidjsonValue(value, jsonArray[i], allocator);
                rapidjsonValue.PushBack(value, allocator);
            }
        }
        break;

    case VALUE_OBJECT:
        {
            const JSONObject& jsonObject = jsonValue.GetObject();

            rapidjsonValue.SetObject();
            for (auto i = jsonObject.begin(); i != jsonObject.end(); ++i)
            {
                const char* name = i->first.c_str();
                rapidjson::Value value;
                ToRapidjsonValue(value, i->second, allocator);
                rapidjsonValue.AddMember(StringRef(name), value, allocator);
            }
        }
        break;

    default:
        break;
    }
}

bool JSONFile::Save(Serializer& dest) const
{
    return Save(dest, "\t");
}

bool JSONFile::Save(Serializer& dest, const String& indendation) const
{
    rapidjson::Document document;
    ToRapidjsonValue(document, root_, document.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(!indendation.empty() ? indendation.front() : '\0', indendation.length());

    document.Accept(writer);
    auto size = (unsigned)buffer.GetSize();
    return dest.Write(buffer.GetString(), size) == size;
}

bool JSONFile::SaveObjectCallback(const std::function<void(Archive&)> serializeValue)
{
    try
    {
        root_.Clear();
        JSONOutputArchive archive{this};
        serializeValue(archive);
        return true;
    }
    catch (const ArchiveException& e)
    {
        root_.Clear();
        //SE_LOG_ERROR("Failed to save object to JSON: {}", e.what());
        return false;
    }
}

bool JSONFile::LoadObjectCallback(const std::function<void(Archive&)> serializeValue) const
{
    try
    {
        JSONInputArchive archive{this};
        serializeValue(archive);
        return true;
    }
    catch (const ArchiveException& e)
    {
        //SE_LOG_ERROR("Failed to load object from JSON: {}", e.what());
        return false;
    }
}

bool JSONFile::FromString(const String& source)
{
    if (source.empty())
        return false;

    MemoryBuffer buffer(source.c_str(), source.length());
    return Load(buffer);
}

String JSONFile::ToString(const String& indendation) const
{
    rapidjson::Document document;
    ToRapidjsonValue(document, root_, document.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(!indendation.empty() ? indendation.front() : '\0', indendation.length());

    document.Accept(writer);
    return buffer.GetString();
}

String ToPrettyString(const JSONValue& json, const String& indendation)
{
    rapidjson::Document document;
    ToRapidjsonValue(document, json, document.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(!indendation.empty() ? indendation.front() : '\0', indendation.length());

    document.Accept(writer);
    return buffer.GetString();
}

}
