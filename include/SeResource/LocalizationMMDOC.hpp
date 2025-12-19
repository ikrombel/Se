
#include <Se/Format.hpp>
#include <SeResource/Localization.h>

namespace MMDOC {

using namespace Se;

class LocalizationBOF : public LocalizationFile
{

public:
    using LocalizationFile::LocalizationFile;


    /// Load resource from custom format.
    bool Load(Deserializer& source, LocalizationFile::LocData& data) override {

        source.Seek(0);

        this->u0 = source.ReadInt(); // 0x02000000
        this->bytesCountText = source.ReadInt(); // loc text size ((std::size_t(this->u2))*sizeof(uint16_t))
        this->u2 = source.ReadInt(); // ??? checksum?

        int i = 0;

        int lenght{0};

        bool isKey{true};
        //String key;
        String var;
        String alt;
        size_t p{};

        // while (!source.IsEof())
        // {
        //     auto keyT = source.ReadWString();
        //     auto key = utf16_to_utf8((uint16_t*)keyT.c_str(), keyT.size());

        //     auto sourceSeek = source.GetPosition();
        //     auto altName = source.ReadWString();

        //     auto prefix = altName.substr(0, 3);
        //     if (   prefix.compare(L"VO_") == 0
        //         || prefix.compare(L"ME_") == 0)
        //     {
        //         //alt = altName; //{var.begin(), var.end()};
        //         //var.clear();
        //     }
        //     else 
        //         source.Seek(sourceSeek);

        //     uint16_t v = 1;
        //     while(v != 0)
        //     {
        //         v = source.ReadShort();

        //         if ((v & 0xFF00) == 0)
        //             var += char(v & 0xFF);
        //         else
        //         {
        //             auto s = utf16_to_utf8(&v, 1);
        //             var += String(s.c_str(), s.size());
        //         }

        //     }

        //     auto value = String(var.c_str(), var.size());
        //     var.clear();
            

        //     auto s = utf16_to_utf8((uint16_t*)value.data(), value.size());
        //     var += String(s.c_str(), s.size());

        //     SE_LOG_PRINT("'{}' {}:\n{}", key, 1, value);

        //     data[key] = std::move(s);
        // }

        auto parseStringUTF16 = [this](Deserializer* d) -> String
        {
            int size = 0;
            uint16_t c = 0;

            //d->Read(&c, 1);
#if 0
            auto pos = d->GetPosition();

            while(!d->IsEof())
            {
                //d->Read(&c, 2);

                c = d->ReadUShort();

                auto pos2 = d->GetPosition();

                if (c == 0)
                    break;
                    
                ++size;
            }

            d->Seek(pos);

            uint16_t buf[size+1];
            d->Read(buf, size*2);
#else
            uint16_t buf[20480];
            while(!d->IsEof())
            {
                c = d->ReadUShort();
                if (c == 0)
                     break;
                if (size < 20480)
                buf[size] = c;
                size++;
            }

            if (size > 20480)
                return "";
#endif
            return utf16_to_utf8(buf, size);
        };

        auto posStart = source.GetPosition();
        std::size_t posCur = 0;
        const std::size_t posEnd = (std::size_t(this->u2))*sizeof(uint16_t);

        int pp = 0;
        while (!source.IsEof() && posCur < posEnd)
        {
            auto key = parseStringUTF16(&source);

            auto value = parseStringUTF16(&source);

            if (value.starts_with("VO_") || value.starts_with("ME_"))
                value = parseStringUTF16(&source);

            
            //int k = 9000;
            //if (pp > k && pp <= k + 200)
            //SE_LOG_PRINT("{}. {}:\n {}", pp, key, value);

            data[std::move(key)] = std::move(value);


            pp++;

            posCur = source.GetPosition() - posStart;
        }

        return true;

        // unsigned chunk2_Size = source.ReadUInt();
        // SE_LOG_DEBUG("chunk2_Size: {}", chunk2_Size);
        // int  chunk2_counter = 0;
        // auto ii = source.ReadUInt();
        // while (!source.IsEof() && chunk2_counter < chunk2_Size*4)
        // {
        //     chunk2_counter++;
        //     char b = source.ReadByte();
        // }

        int tt = 0;
        //int offsetTmp = 30000;
        while (!source.IsEof())
        {
            tt++;
#if 0
            char b = source.ReadByte();
            continue;
#else
            unsigned char b[32];
            unsigned char b_str[32];
            source.Read(b, 32);

            // if (tt < offsetTmp || tt > offsetTmp + 32)
            //     continue;

            if (tt > 32)
                break;


            for(int i =0; i < 32; i++)
            {
                char c = b[i];
                if ((c > 'A' && c < 'Z') || (c > 'a' && c < 'z'))
                    b_str[i] = b[i];
                else
                    b_str[i] = (c == 0) ? ' ' : '?';

            }

            std::string s = String((char*)b, 32);
            SE_LOG_PRINT(cformat(
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X\n"
                "%02c %02c %02c %02c %02c %02c %02c %02c "
                "%02c %02c %02c %02c %02c %02c %02c %02c "
                "%02c %02c %02c %02c %02c %02c %02c %02c "
                "%02c %02c %02c %02c %02c %02c %02c %02c\n",
                b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
                b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15],
                b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23],
                b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31],
                b_str[0],  b_str[1], b_str[2], b_str[3], b_str[4], b_str[5], b_str[6], b_str[7],
                b_str[8],  b_str[9],  b_str[10], b_str[11], b_str[12], b_str[13], b_str[14], b_str[15],
                b_str[16], b_str[17], b_str[18], b_str[19], b_str[20], b_str[21], b_str[22], b_str[23],
                b_str[24], b_str[25], b_str[26], b_str[27], b_str[28], b_str[29], b_str[30], b_str[31]
                ).c_str());
#endif
        }

        // SE_LOG_INFO("{} SIZE: {}", source.GetName(), data.size());
        SE_LOG_INFO("p: {}\tu1:{}\tu2: {}", posCur, this->bytesCountText, this->u2);
        SE_LOG_PRINT("size: {}", std::size_t(this->u2) + this->bytesCountText);
        SE_LOG_PRINT("tt: {}", tt);

        //SE_LOG_INFO("psize: {}", source.GetSize()-this->u2);
        return true;

    }

    /// Save resource to custom format.
    bool Save(Serializer& source, const LocalizationFile::LocData& data) const override { 
        return true; }



private:
    int u0{};
    int bytesCountText{};
    int u2{};
};


// class LangBof : public ResourceSerializable
// {
//     GFROST_OBJECT(LangBof, ResourceSerializable);
// public:

//     LangBof(Context* context) : ResourceSerializable(context) {
//         //abilities_.push_back(MakeShared<Ability>(context));

//         vars_["_"] = "Test";
//     }

//     const StringVariantMap GetVars() const { return vars_; }
    
//     void SetVars(const StringVariantMap& vars) { vars_ = vars; }

//     static void RegisterObject(Context* context) {

//         context->AddFactoryReflection<LangBof>();
//         GFROST_COPY_BASE_ATTRIBUTES(ResourceSerializable);
//         GFROST_ATTRIBUTE("u0", int, u0, 0, AM_DEFAULT);
//         GFROST_ATTRIBUTE("u1", int, u1, 0, AM_DEFAULT);
//         //GFROST_ATTRIBUTE("Size", int, u2, 0, AM_DEFAULT);
//         GFROST_ACCESSOR_ATTRIBUTE("Vars", GetVars, SetVars, StringVariantMap, Variant::emptyStringVector, AM_DEFAULT);
//     }

//     int u0{};
//     int u1{};
//     int u2{};

// private:
//     StringVariantMap vars_;

// };

}