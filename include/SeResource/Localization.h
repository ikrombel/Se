#pragma once

#include <Se/Signal.hpp>
#include <Se/StringHash.hpp>
#include <Se/IO/AbstractFile.hpp>
#include <SeResource/JSONValue.h>

#include <type_traits>

namespace Se
{


class Localization;

/// This is an optional feature for Serialize/Deserialize user's file format.
class LocalizationFile
{
public:

    //using LocData = std::unordered_map<StringHash, std::unordered_map<StringHash, String>>;
    
#if SE_EDITOR
    using LocData = std::unordered_map<String, String>;
#else
    using LocData = std::unordered_map<StringHash, String>;
#endif
    

    explicit LocalizationFile(const Localization* localization) 
    : localization_(const_cast<Localization*>(localization)) {}

    /// Load resource from custom format.
    virtual bool Load(Deserializer& source, LocData& data) { 
        return false; }

     /// Save resource to custom format.
    virtual bool Save(Serializer& source, const LocData& data) const { 
        return false; }

protected:
    //
    std::string utf16_to_utf8(const uint16_t* utf16_str, size_t length);
    // 
    std::vector<uint16_t> utf8_to_utf16(const std::string& utf8_str);

    String parseStringUTF16(Deserializer& d);


    Localization* localization_{nullptr};
};

/// %Localization subsystem. Stores all the strings in all languages.
class Localization
{

public:
    /// Language changed. E_CHANGELANGUAGE
    static Signal<> onChangeLanguage;

    /// Construct.
    explicit Localization();
    /// Destruct. Free all resources.
    virtual ~Localization();

    /// Return the number of languages.
    /// @property
    int GetNumLanguages() const { return (int)languages_.size(); }

    /// Return the index number of current language. The index is determined by the order of loading.
    /// @property
    int GetLanguageIndex() const { return languageIndex_; }

    /// Return the index number of language. The index is determined by the order of loading.
    int GetLanguageIndex(const String& language);
    /// Return the name of current language.
    /// @property
    String GetLanguage();
    /// Return the name of language.
    String GetLanguage(int index);
    /// Set current language.
    void SetLanguage(int index);
    /// Set current language.
    void SetLanguage(const String& language);
    /// Return a string in the current language. Returns String::EMPTY if id is empty. Returns id if translation is not found and logs a warning.
    String Get(const String& id);
    /// Clear all loaded strings.
    void Reset();
    /// Load strings from JSONFile. The file should be UTF8 without BOM.
    void LoadJSONFile(const String& name, const String& language = String::EMPTY);
    /// Load strings from JSONValue.
    void LoadMultipleLanguageJSON(const JSONValue& source);
    /// Load strings from JSONValue for specific language.
    void LoadSingleLanguageJSON(const JSONValue& source, const String& language = String::EMPTY);

    template<class T = LocalizationFile>
    bool Load(Deserializer& source, const String& lang)
    {
        auto seekBackup = source.GetPosition();

        if (std::find(languages_.begin(), languages_.end(), lang) == languages_.end())
            languages_.push_back(lang);
        
        auto file = std::shared_ptr<LocalizationFile>(new T(this));
        //files_[lang] = file;
        auto& strings = strings_[lang];
        bool load = file->Load(source, strings);

        if (load)
            return true;

        source.Seek(seekBackup);  
        return false;    
    }

    template<class T = LocalizationFile>
    bool Save(Serializer& source, const String& lang) const
    {
        if (std::find(languages_.begin(), languages_.end(), lang) == languages_.end())
            return false;

        //auto file = std::make_unique<T>(this);
        auto file = std::shared_ptr<LocalizationFile>(new T(this));
        auto& strings = strings_.at(lang);
        return file->Save(source, strings);
    }

private:
    /// Language names.
    std::vector<String> languages_;
    /// Index of current language.
    int languageIndex_;
    /// Storage strings: <Language <StringId, Value> >.
    std::unordered_map<StringHash, LocalizationFile::LocData> strings_;

    //std::unordered_map<StringHash, std::shared_ptr<Se::LocalizationFile>> files_;
};

}