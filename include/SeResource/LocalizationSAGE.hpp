#include <SeResource/Localization.h>
#include <algorithm>
#include <string.h>

#ifndef stricmp
#define stricmp strcasecmp
#endif

namespace SAGE
{

using Se::String;
using Se::Deserializer;
using Se::Serializer;
using Se::LocalizationFile;

class LocalizationSTR : public LocalizationFile
{

public:
    using LocalizationFile::LocalizationFile;

    /// Load resource from custom format.
    bool Load(Deserializer& source, LocalizationFile::LocData& data) override
    {
        
        data.clear();
        
        bool ok = false;
        int textCount = 0;

        String key;

        auto is_special_char = [](char c) { 
            return std::isspace(c) || c == '\t' || c == '\n'; 
        };
    
        while (!source.IsEof())
        {
            auto line = source.ReadLine();
            std::remove_if(line.begin(), line.end(), is_special_char);

            if (line.empty())
                continue;
            else
            {
                //removeLeadingAndTrailing(line.data());
                key = line;
            }

            String value;
            line = source.ReadLine();

            if (stricmp(line.data(), "END") == 0)
            {
                data.emplace(std::move(key), format("EMPTY_KEY: {}", key));
                continue;
            }

            if (line.front() == '"')
            {
                value = line;

                while (!source.IsEof())
                {
                    auto lineValue = source.ReadLine();

                    if (stricmp(lineValue.data(), "END") == 0)
                    {

                        // \TODO: use in parser
                        //value.replace("\\n", "\n");

                        if (value.size() == 2 && value.front() == '"' && value.back() == '"') {
                            data.emplace(std::move(key), format("EMPTY_KEY: {}", key));
                            break;
                        }

                        if (value.back() == '"' /*&& !(value.length() >= 2 && (*(value.end()-2) == '\\'))*/)
                            value.pop_back();

                        value.pop_front();
                        
                        textCount++;
                        //
                        data.emplace(std::move(key), std::move(value));
                        break;
                    }
                    value.append(lineValue);
                }
            }
        }

        return true; 
    }

     /// Save resource to custom format.
    bool Save(Serializer& source, const LocalizationFile::LocData& data) const override { 
        return true; }   
};

#define CSF_ID ( ('C'<<24) | ('S'<<16) | ('F'<<8) | (' ') )
#define CSF_LABEL ( ('L'<<24) | ('B'<<16) | ('L'<<8) | (' ') )
//#define CSF_STRING ( ('S'<<24) | ('T'<<16) | ('R'<<8) | (' ') )
//#define CSF_STRINGWITHWAVE ( ('S'<<24) | ('T'<<16) | ('R'<<8) | ('W') )
//#define CSF_VERSION 3

class LocalizationCSV : public LocalizationFile
{


public:
    using LocalizationFile::LocalizationFile;


    /// Load resource from custom format.
    bool Load(Deserializer& source, LocalizationFile::LocData& data) override { 

        data.clear();

        int countTest = 0;

        int id{};
        int len{};

        int m_maxLabelLen = 0;

        auto pos = source.GetPosition();

        CSFHeader header;
        if (source.Read(&header, sizeof(header)) != sizeof(header))
        {
            source.Seek(pos);
            return false;
        }

        if (   header.id != CSF_ID 
            || localization_->GetLanguage(header.langid).empty())
        {
            source.Seek(pos);
            return false;
        }

        bool ok = false;
	    //wchar_t m_tbuffer[MAX_UITEXT_LENGTH*2];

        
        for (uint32_t i = 0; i < header.num_labels; i++)
        {
            //LangInfo langInfo;

            source.Read(&id, sizeof(id));

            if (id != CSF_LABEL)
                return false;

            uint32_t countOfStrings;
            source.Read(&countOfStrings, sizeof(countOfStrings));

            uint32_t labelNameLength;
            source.Read(&labelNameLength, sizeof(labelNameLength));

            String key;
            String value;
            if (labelNameLength)
            {
                key.resize(labelNameLength);
                source.Read(key.data(), labelNameLength);
                //langInfo.name = std::move(key);
            }

            if (countOfStrings != 0)
            {

                // Read string type
                uint8_t  rtsOrWrts[4];
                source.Read(reinterpret_cast<char*>(&rtsOrWrts), sizeof(rtsOrWrts));

                // Read string lenght
                uint32_t lenS;
                source.Read(reinterpret_cast<char*>(&lenS), sizeof(lenS));

                // Read byte-like compiled string
                //uint16_t wchBufferValue[len];
                std::vector<uint16_t> wchBufferValue(lenS);
                source.Read((void*)wchBufferValue.data(), lenS * sizeof(uint16_t));

                // Reverse read string
                for (int tmp = 0; tmp < lenS; tmp++)
                {
                    wchBufferValue[tmp] = ~wchBufferValue[tmp];
                }

                try
                {
                    //langInfo.text = utf16_to_utf8(wchBufferValue.data(), lenS);
                    value = utf16_to_utf8(wchBufferValue.data(), lenS);
                }
                catch(const std::exception& e)
                {
                    SE_LOG_ERROR("Exception: {}\nLocalizationCSV::Load: error convert utf16 to utf8 for key '{}'"
                            , e.what(), key);

                    for (int tmp = 0; tmp < lenS; tmp++)
                    {
                        //langInfo.text += static_cast<char>(wchBufferValue[tmp] & 0xFF);
                        value += static_cast<char>(wchBufferValue[tmp] & 0xFF);
                    }
                    // SE_LOG_PRINT("{}", langInfo.text );
                    SE_LOG_PRINT("{}", value );
                }

                wchBufferValue.clear();
                
                


                // Read extra value and do not write bcs it's useless
                if((char)rtsOrWrts[0] == 'W')
                {
                    uint32_t extraValueLength;
                    source.Read(reinterpret_cast<char*>(&extraValueLength), sizeof(extraValueLength));

                    uint8_t extraValue[extraValueLength];
                    source.Read(reinterpret_cast<char*>(&extraValue), sizeof(extraValue));
                    //langInfo.speech = std::string(reinterpret_cast<char*>(extraValue), extraValueLength);
                }

                data[std::move(key)] = std::move(value);

                countTest++;
            }
        //     printf("______ %s\n", key.c_str());
		// if (langInfo.text.length() > 0)
		// 	printf("--- tx\n%s\n", langInfo.text.c_str());
		// if (langInfo.speech.length() > 0)
		// 	printf("--- sp\n%s\n", langInfo.speech.c_str());
        }



        return true; 
    }

     /// Save resource to custom format.
    bool Save(Serializer& source, const LocalizationFile::LocData& data) const override { 
        return true; } 
        
private:
    struct CSFHeader
    {
        int id;
        int version;
        int num_labels;
        int num_strings;
        int skip;
        int langid;
    };

    struct LangInfo
    {
        std::string name;
        //std::string waveFile;
        std::string text;
        std::string	speech;
    };

    static constexpr int MAX_UITEXT_LENGTH = 10*1024;

};
    
} // namespace SAGE