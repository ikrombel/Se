#include "LangFile.h"

#include <Se/IO/File.h>

#include <string.h>
#include <algorithm>
#include <vector>

#include <locale>
#include <codecvt>
#include <type_traits>

#ifndef stricmp
#define stricmp strcasecmp
#endif

#ifdef _WIN32
#include <cinttypes>
#endif

namespace Se {

#define CSF_ID ( ('C'<<24) | ('S'<<16) | ('F'<<8) | (' ') )
#define CSF_LABEL ( ('L'<<24) | ('B'<<16) | ('L'<<8) | (' ') )
#define CSF_STRING ( ('S'<<24) | ('T'<<16) | ('R'<<8) | (' ') )
#define CSF_STRINGWITHWAVE ( ('S'<<24) | ('T'<<16) | ('R'<<8) | ('W') )
#define CSF_VERSION 3

struct CSFHeader
{
	int id;
	int version;
	int num_labels;
	int num_strings;
	int skip;
	int langid;
};

// void removeLeadingAndTrailing (char *buffer)
// {
// 	char *first, *ptr;
// 	char ch;

// 	ptr = first = buffer;

// 	while ( (ch = *first) != 0 && iswspace ( ch ))
// 	{
// 			first++;
// 	}

// 	while ( (*ptr++ = *first++) != 0 );

// 	ptr -= 2;;

// 	while ( (ptr > buffer) && (ch = *ptr) != 0 && iswspace ( ch ) )
// 	{
// 		ptr--;
// 	}

// 	ptr++;
// 	*ptr = 0;
// }

// void readToEndOfQuote(Deserializer& file, char *in, char *out, char *wavefile, int maxBufLen )
// {
// 	int slash = false;
// 	int state = 0;
// 	int line_start = false;
// 	char ch;
// 	int ccount = 0;
// 	int len = 0;
// 	int done = false;

// 	while (maxBufLen)
// 	{
// 		// get next char

// 		if ( in )
// 		{
// 			if ( (ch = *in++) == 0 )
// 			{
// 				in = NULL; // have exhausted the input m_buffer
// 				ch = file.ReadByte();
// 			}
// 		}
// 		else
// 		{
// 			ch = file.ReadByte();
// 		}

// 		if ( ch == EOF )
// 		{
// 			return ;
// 		}

// 		if ( ch == '\n' )
// 		{
// 			line_start = true;
// 			slash = false;
// 			ccount = 0;
// 			ch = ' ';
// 		}
// 		else if ( ch == '\\' && !slash)
// 		{
// 			slash = true;
// 		}
// 		else if ( ch == '\\' && slash)
// 		{
// 			slash = false;
// 		}
// 		else if ( ch == '"' && !slash )
// 		{
// 			break; // done
// 		}
// 		else
// 		{
// 			slash = false;
// 		}

// 		if ( iswspace ( ch ))
// 		{
// 			ch = ' ';
// 		}

// 		*out++ = ch;
// 		maxBufLen--;
// 	}

// 	*out = 0;

// 	while ( !done )
// 	{
// 		// get next Char

// 		if ( in )
// 		{
// 			if ( (ch = *in++) == 0 )
// 			{
// 				in = NULL; // have exhausted the input m_buffer
// 				ch = file.ReadByte();
// 			}
// 		}
// 		else
// 		{
// 			ch = file.ReadByte();
// 		}

// 		if ( ch == '\n' || ch == EOF )
// 		{
// 			break;
// 		}

// 		switch ( state )
// 		{

// 			case 0:
// 				if ( iswspace ( ch ) || ch == '=' )
// 				{
// 					break;
// 				}

// 				state = 1;
// 			case 1:
// 				if ( ( ch >= 'a' && ch <= 'z') || ( ch >= 'A' && ch <='Z') || (ch >= '0' && ch <= '9') || ch == '_' )
// 				{
// 					*wavefile++ = ch;
// 					len++;
// 					break;
// 				}
// 				state = 2;
// 			case 2:
// 				break;
// 		}
// 	}

// 	*wavefile = 0;

// 	if ( len )
// 	{
// 		if ( ( ch = *(wavefile-1)) >= '0' && ch <= '9' )
// 		{
// 			*wavefile++ = 'e';
// 			*wavefile = 0;
// 		}
// 	}

// }

struct LangInfo
{
    std::string name;
    //std::string waveFile;
    std::string text;
	std::string	speech;
};

#define MAX_UITEXT_LENGTH (10*1024)

bool LangFile::ReadCSFHeader(Deserializer& source, CSFHeader& header)
{
	if (source.Read(&header, sizeof(header)) == sizeof(header) || header.id == CSF_ID) // CSF ID
    {
		return LoadCSF(source);
        return true;
    }

	return false;
}

bool LangFile::BeginLoad(Deserializer& source)
{

	CSFHeader header;
    if (source.Read(&header, sizeof(header)) == sizeof(header) || header.id == CSF_ID) // CSF ID
    {
		return LoadCSF(source);
    }

	source.Seek(0);
    if (LoadStr(source))
    {
        onReloadFinished();
        return true;
    }

    onReloadFailed();
    return false;
}



// void stripSpaces ( wchar_t *string )
// {
// 	wchar_t *str, *ptr;
// 	wchar_t ch, last = 0;
// 	int skipall = true;

// 	str = ptr = string;

// 	while ((ch = *ptr++) != 0)
// 	{
// 		if (ch == ' ')
// 		{
// 			if (last == ' ' || skipall)
// 			{
// 				continue;
// 			}
// 		}

// 		if ( ch == '\n' || ch == '\t' )
// 		{
// 				// remove last space
// 				if (last == ' ')
// 				{
// 					str--;
// 				}

// 				skipall = true;		// skip all spaces
// 				last = *str++ = ch;
// 				continue;
// 		}

// 		last = *str++ = ch;
// 		skipall = false;
// 	}

// 	if ( last == ' ' )
// 	{
// 		str--;
// 	}

// 	*str = 0;
// }

std::string utf16_to_utf8(const uint16_t* utf16_str, size_t length) {
    // Create a std::wstring from the uint16_t array
    std::wstring wstr(utf16_str, utf16_str + length);
    
    // Create a converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    // Convert UTF-16 to UTF-8
    return converter.to_bytes(wstr);
}

std::vector<uint16_t> utf8_to_utf16(const std::string& utf8_str) {
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

bool LangFile::LoadCSF(Deserializer& source)
{
    translations_.clear();

	//for log
	int countTest = 0;

    int id{};
    int len{};

	int m_maxLabelLen = 0;

	wchar_t m_tbuffer[MAX_UITEXT_LENGTH*2];

	source.Seek(0);

    CSFHeader header;
    if (source.Read(&header, sizeof(header)) != sizeof(header) || header.id != CSF_ID) // CSF ID
    {
        return false;
    }

	//m_maxLabelLen = header.num_labels;

    if (header.version >= 2)
        languageID_ = header.langid;
    else
        languageID_ = 0; // LANGUAGE_ID_US

    bool ok = false;

	for (uint32_t i = 0; i < header.num_labels; i++)
	{
		LangInfo langInfo;

		source.Read(&id, sizeof(id));

		if (id != CSF_LABEL)
			return false;

		uint32_t countOfStrings;
		source.Read(&countOfStrings, sizeof(countOfStrings));

		uint32_t labelNameLength;
		source.Read(&labelNameLength, sizeof(labelNameLength));

		std::string key;
		if (labelNameLength)
		{
			key.resize(labelNameLength);
			source.Read(key.data(), labelNameLength);
			langInfo.name = key;
		}

		if (countOfStrings != 0)
		{
			// Read string type
			uint8_t  rtsOrWrts[4];
			source.Read(reinterpret_cast<char*>(&rtsOrWrts), sizeof(rtsOrWrts));

			// Read string lenght
			uint32_t len;
			source.Read(reinterpret_cast<char*>(&len), sizeof(len));

			// Read byte-like compiled string
			uint16_t wchBufferValue[len];
			source.Read((void*)wchBufferValue, std::size_t(len) * sizeof(uint16_t));

			// Reverse read string
			for (int tmp = 0; tmp < len; tmp++)
			{
				wchBufferValue[tmp] = ~wchBufferValue[tmp];
			}


			langInfo.text = utf16_to_utf8(wchBufferValue, len);

			// Read extra value and do not write bcs it's useless
			if((char)rtsOrWrts[0] == 'W')
			{
				uint32_t extraValueLength;
				source.Read(reinterpret_cast<char*>(&extraValueLength), sizeof(extraValueLength));

				uint8_t extraValue[extraValueLength];
				source.Read(reinterpret_cast<char*>(&extraValue), sizeof(extraValue));
				langInfo.speech = std::string(reinterpret_cast<char*>(extraValue), extraValueLength);
			}

			countTest++;
				
		}
		printf("______ %s\n", key.c_str());
		if (langInfo.text.length() > 0)
			printf("--- tx\n%s\n", langInfo.text.c_str());
		if (langInfo.speech.length() > 0)
			printf("--- sp\n%s\n", langInfo.speech.c_str());
	}

    return true;
}

bool LangFile::LoadStr(Deserializer& source)
{
    translations_.clear();

    bool ok = false;
    int textCount = 0;

    String key;
    
    while (!source.IsEof())
    {
        auto line = source.ReadLine();
        //line.replace(" ", "");
        std::remove_if(line.begin(), line.end(), [](char c) { 
                return std::isspace(c) || c == '\t' || c == '\n'; 
            });

        if (line.empty())
            continue;
        else
        {
            //removeLeadingAndTrailing(line.data());
            key = line;
        }

        String value;
        line = source.ReadLine();
        if (line.front() == '"')
		{
            value = line;
            value.pop_front();

            while (!source.IsEof())
            {
                auto lineValue = source.ReadLine();

                if (!stricmp(lineValue.data(), "END"))
                {
                    if (line.back() == '"')
                        value.pop_back();
                    value.replace("\\n", "\n");
                    textCount++;
                    //
                    translations_.emplace(std::move(key), std::move(value));
                    break;
                }
                value.append(lineValue);
            }
		}
    }

    return true;
}



    // /// Finish resource loading. Always called from the main thread. Return true if successful.
    // virtual bool EndLoad();
    /// Save resource. Return true if successful.
bool LangFile::Save(Serializer& dest) const
{
    

    return false;
}

}