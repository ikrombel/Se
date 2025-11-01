#pragma once

#include <string>
#include <unordered_map>

#include <SeResource/Resource.h>


namespace Se
{

struct CSFHeader;

class SE_API LangFile : public Resource
{
public:

    LangFile() 
        : Resource("LangFile")
    {
    }


    //LangFile(const std::string& filePath);
    ~LangFile()
    {
        translations_.clear();
    }

    inline static String GetTypeStatic() { return "LangFile"; }

    bool Load();
    void Unload();

    Se::String GetTranslation(const std::string& key) const { 
        return translations_.at(key); }

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    virtual bool BeginLoad(Deserializer& source);
    // /// Finish resource loading. Always called from the main thread. Return true if successful.
    // virtual bool EndLoad();
    /// Save resource. Return true if successful.
    virtual bool Save(Serializer& dest) const;

private:
    /// Load resource from string format.
    bool LoadStr(Deserializer& source);
    /// Load resource from CSV format.
    bool ReadCSFHeader(Deserializer& source, CSFHeader& header);
    bool LoadCSF(Deserializer& source);

    std::string m_filePath;
    std::unordered_map<std::string, Se::String> translations_;
    std::vector<std::string> keys_;

    int languageID_{};
};

} // namespace Se
