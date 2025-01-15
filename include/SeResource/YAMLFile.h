#pragma once

#include <SeResource/Resource.h>
#include <SeResource/JSONValue.h>

// namespace ryml {
//     class Tree;
// }

namespace Se
{
    
    class YAMLFile : public Resource
    {
    public:
        YAMLFile() : Resource("YAMLFile") {}
        virtual ~YAMLFile() = default;

        inline static String GetTypeStatic() { return "YAMLFile"; }

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        bool BeginLoad(Deserializer& source) override;
        /// Save resource with default indentation (one tab). Return true if successful.
        bool Save(Serializer& dest) const override;

        // ryml::Tree& GetTree()
        // {
        //     return m_tree;
        // }

        String ToString() const;

        JSONValue& GetRoot()
        {
            return value_;
        }

    private:
        //ryml::Tree m_tree;
        JSONValue value_;
    };
}


// bool Save(const std::string& filePath)
// {
//     std::ofstream file(filePath);
//     if (!file.is_open())
//     {
//         return false;
//     }

//     std::string content;
//     ryml::emit(m_tree, &content);
//     file << content;

//     return true;
// }