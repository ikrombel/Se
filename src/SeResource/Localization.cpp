
#include <Se/Console.hpp>

#include "Localization.h"
#include "ResourceCache.h"
#include "JSONFile.h"
//#include "ResourceEvents.h"

//#include <GFrost/DebugNew.h>

#include <codecvt>

namespace Se
{

std::string LocalizationFile::utf16_to_utf8(const uint16_t* utf16_str, size_t length) {
    // Create a std::wstring from the uint16_t array
    std::wstring wstr(utf16_str, utf16_str + length);
    
    // Create a converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    // Convert UTF-16 to UTF-8
    return converter.to_bytes(wstr);
}

std::vector<uint16_t> LocalizationFile::utf8_to_utf16(const std::string& utf8_str) {
    // Create a converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    // Convert UTF-8 to UTF-16 (std::wstring)
    std::wstring wstr = converter.from_bytes(utf8_str);
    
    // Create a vector of uint16_t from the std::wstring
    std::vector<uint16_t> utf16_vec(wstr.begin(), wstr.end());
    
    // Add a null terminator (optional, depending on your use case)
    utf16_vec.push_back(0);
    
    return utf16_vec;
}

String LocalizationFile::parseStringUTF16(Deserializer& d)
{

    uint16_t c = 1;

    auto pos = d.GetPosition();

    int size = 0;


    uint16_t buf[204800];

    while(!d.IsEof())
    {
        c = d.ReadUShort();
        if (c == 0)
            break;
        if (size < 20480)
        buf[size] = c;
        size++;
    }

    if (size > 204800)
        return "";

    //d.Seek(pos);

    
    //d.Read(buf, size*2);

    return utf16_to_utf8(buf, size);
};


Signal<> Localization::onChangeLanguage;

Localization::Localization() :
    languageIndex_(-1)
{
}

Localization::~Localization()
{
    languages_.clear();
};

int Localization::GetLanguageIndex(const String& language)
{
    if (language.empty())
    {
        SE_LOG_WARNING("Localization::GetLanguageIndex(language): language name is empty");
        return -1;
    }
    if (GetNumLanguages() == 0)
    {
        SE_LOG_WARNING("Localization::GetLanguageIndex(language): no loaded languages");
        return -1;
    }
    for (int i = 0; i < GetNumLanguages(); i++)
    {
        if (languages_[i] == language)
            return i;
    }
    return -1;
}

String Localization::GetLanguage()
{
    if (languageIndex_ == -1)
    {
        SE_LOG_WARNING("Localization::GetLanguage(): no loaded languages");
        return String::EMPTY;
    }
    return languages_[languageIndex_];
}

String Localization::GetLanguage(int index)
{
    if (GetNumLanguages() == 0)
    {
        SE_LOG_WARNING("Localization::GetLanguage(index): no loaded languages");
        return String::EMPTY;
    }
    if (index < 0 || index >= GetNumLanguages())
    {
        SE_LOG_WARNING("Localization::GetLanguage(index): index out of range");
        return String::EMPTY;
    }
    return languages_[index];
}

void Localization::SetLanguage(int index)
{
    if (GetNumLanguages() == 0)
    {
        SE_LOG_WARNING("Localization::SetLanguage(index): no loaded languages");
        return;
    }
    if (index < 0 || index >= GetNumLanguages())
    {
        SE_LOG_WARNING("Localization::SetLanguage(index): index out of range");
        return;
    }
    if (index != languageIndex_)
    {
        languageIndex_ = index;
        onChangeLanguage();
        // VariantMap& eventData = GetEventDataMap();
        // SendEvent(E_CHANGELANGUAGE, eventData);
    }
}

void Localization::SetLanguage(const String& language)
{
    if (language.empty())
    {
        SE_LOG_WARNING("Localization::SetLanguage(language): language name is empty");
        return;
    }
    if (GetNumLanguages() == 0)
    {
        SE_LOG_WARNING("Localization::SetLanguage(language): no loaded languages");
        return;
    }
    int index = GetLanguageIndex(language);
    if (index == -1)
    {
        SE_LOG_WARNING("Localization::SetLanguage(language): language not found");
        return;
    }
    SetLanguage(index);
}

String Localization::Get(const String& id)
{
    if (id.empty())
        return String::EMPTY;
    if (GetNumLanguages() == 0)
    {
        SE_LOG_WARNING("Localization::Get(id): no loaded languages");
        return id;
    }
    String result = strings_[{GetLanguage()}][{id}];
    if (result.empty())
    {
        SE_LOG_WARNING("Localization::Get(\"" + id + "\") not found translation, language=\"" + GetLanguage() + "\"");
        return id;
    }
    return result;
}

void Localization::Reset()
{
    languages_.clear();
    languageIndex_ = -1;
    strings_.clear();
}

void Localization::LoadJSONFile(const String& name, const String& language)
{
    auto& cache = ResourceCache::Get();
    auto* jsonFile = cache.GetResource<JSONFile>(name);
    if (jsonFile)
    {
        if (language.empty())
            LoadMultipleLanguageJSON(jsonFile->GetRoot());
        else
            LoadSingleLanguageJSON(jsonFile->GetRoot(), language);
    }
}


void Localization::LoadMultipleLanguageJSON(const JSONValue& source)
{
    for (auto i = source.begin(); i != source.end(); ++i)
    {
        String id = i->first;
        if (id.empty())
        {
            SE_LOG_WARNING("Localization::LoadMultipleLanguageJSON(source): string ID is empty");
            continue;
        }
        const JSONValue& value = i->second;
        if (value.IsObject())
        {
            for (auto j = value.begin(); j != value.end(); ++j)
            {
                const String &lang = j->first;
                if (lang.empty())
                {
                    SE_LOG_WARNING(
                            "Localization::LoadMultipleLanguageJSON(source): language name is empty, string ID=\"{}\"", id);
                    continue;
                }
                const String &string = j->second.GetString();
                if (string.empty())
                {
                    SE_LOG_WARNING(
                            "Localization::LoadMultipleLanguageJSON(source): translation is empty, "
                            "string ID=\"{}\", language=\"{}\"", id, lang);
                    continue;
                }
                if (strings_[{lang}][{id}] != String::EMPTY)
                {
                    SE_LOG_WARNING(
                            "Localization::LoadMultipleLanguageJSON(source): override translation, "
                            "string ID=\"{}\", language=\"{}\"", id, lang);
                }
                strings_[{lang}][{id}] = string;
                if (std::find(languages_.begin(), languages_.end(), lang) == languages_.end())
                    languages_.push_back(lang);
                if (languageIndex_ == -1)
                    languageIndex_ = 0;
            }
        }
        else
            SE_LOG_WARNING("Localization::LoadMultipleLanguageJSON(source): failed to load values, string ID=\"" + id + "\"");
    }
}

void Localization::LoadSingleLanguageJSON(const JSONValue& source, const String& language)
{
    for (auto i = source.begin(); i != source.end(); ++i)
    {
        String id = i->first;
        if (id.empty())
        {
            SE_LOG_WARNING("Localization::LoadSingleLanguageJSON(source, language): string ID is empty");
            continue;
        }
        const JSONValue& value = i->second;
        if (value.IsString())
        {
            if (value.GetString().empty())
            {
                SE_LOG_WARNING(
                        "Localization::LoadSingleLanguageJSON(source, language): translation is empty, "
                        "string ID=\"{}\", language=\"{}\"", id, language);
                continue;
            }
            if (strings_[{language}][{id}] != String::EMPTY)
            {
                SE_LOG_WARNING(
                        "Localization::LoadSingleLanguageJSON(source, language): override translation, "
                        "string ID=\"{}\", language=\"{}\"", id, language);
            }
            strings_[{language}][{id}] = value.GetString();
            if (std::find(languages_.begin(), languages_.end(), language) == languages_.end())
                languages_.push_back(language);
        }
        else
            SE_LOG_WARNING(
                    "Localization::LoadSingleLanguageJSON(source, language): failed to load value, "
                    "string ID=\"{}\", language=\"{}\"", id, language);
    }
}

}
