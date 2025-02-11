#include <SeResource/YAMLFile.h>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml-0.7.2/rapidyaml.hpp"

#include <Se/Console.hpp>
#include <Se/IO/AbstractFile.hpp>
#include <SeResource/JSONValue.h>
#include <SeResource/JSONFile.h> //debug

namespace Se
{

#if _WIN32
float strtof32(const char* str, char** end) {
    return static_cast<float>(strtof(str, end));
}
#endif

JSONValue ToJSONValue(const ryml::ConstNodeRef& node)
{
    if (node.has_key())
    auto tmpKey = node.key().str;

    JSONValue value;
    if (node.is_map())
    {
        for (auto child : node.children())
        {
            auto keyTmp = String(child.key().str, child.key().size());
            value.Set(keyTmp, ToJSONValue(child));
        }
    }
    else if (node.is_seq())
    {
        for (auto child : node.children())
        {
            value.Push(ToJSONValue(child));
        }
    }
    else if(node.key_is_null() && node.val_is_null())
    {

    }
    else if (node.is_keyval())
    {  
        auto yamlValue = node.val();
        JSONValue tmpValue = JSONValue::EMPTY;

        // Handle primitive types
        if (yamlValue.is_number() && yamlValue.is_integer()) {
            tmpValue = yamlValue.str ? (int)strtol(yamlValue.str, nullptr, 10) : 0;
        }
        else if (yamlValue.is_number() && yamlValue.is_real()) {
            tmpValue = yamlValue.str ? (float)strtof32(yamlValue.str, nullptr) : 0;

            //value = yamlValue.float Set(node.first, node.second.val());
        } 
        // else if (yamlValue.is_real()) {
        //     value = ToBool(yamlValue.str);
        //     //jsonValue.Set(node.first, node.second.val());
        // }
        else {
            if (yamlValue.str)
                tmpValue = yamlValue.str ? String(yamlValue.str, yamlValue.size()) : "";
        }

        value = tmpValue;
    }

    else if (node.has_val())
    {
        value = JSONValue::EMPTY;
    }
    return value;
}

void ToYml(ryml::NodeRef* node, const JSONValue& input)
{
    if (input.IsObject())
    {
        *node |= ryml::MAP;

        for (auto& [key, value] : input.GetObject())
        {
            auto tempKey = key.c_str();

            //ryml::NodeRef child = ryml::NodeRef();
            
            ryml::NodeRef child = node->append_child();
            //ryml::NodeRef child = node[key.c_str()];
            child.set_key(ryml::csubstr(key.c_str()));

            //auto child = node[key.c_str()];
            ToYml(&child, value);
            (*node)[key.c_str()] = child; // << ryml::key(key);
            //node.append_child() = (child);
        }
    }
    else if (input.IsArray())
    {
        if (!node->is_root())
            *node |= ryml::SEQ;
        for (auto& value : input.GetArray())
        {
            ryml::NodeRef child = node->append_child();
            ToYml(&child, value);
            node->append_child() = (child);
        }
    }
    else if (input.IsString())
    {
        node->set_val(input.GetString().c_str());
    }
    else if (input.IsNumber() && input.GetNumberType() == JSONNumberType::JSONNT_INT)
    {
        node->set_val(cformat("%i", input.GetInt()).c_str());
    }
    else if (input.IsNumber() && input.GetNumberType() == JSONNumberType::JSONNT_FLOAT_DOUBLE)
    {
        node->set_val(cformat("%d", input.GetFloat()).c_str());
    }
    // child |= ryml::FLOW_SL; // flow, single-line
    else if (input.IsBool())
    {
        node->set_val(cformat("%i", input.GetBool() ? 1 : 0).c_str());
    }
}

bool YAMLFile::BeginLoad(Deserializer& source) {
    
    String content = source.ReadString();
    ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(content));
    value_ = ToJSONValue(tree.crootref());


    // JSONFile file;
    // file.GetRoot() = value_;
    // SE_LOG_INFO("test:\n{}",file.ToString("  "));

    return true;
}

String YAMLFile::ToString() const
{
#ifdef TODO
    std::string content;
    //content.reserve(10000);

    ryml::Tree tree;

    tree.reserve(10);         // reserve the number of nodes
    tree.reserve_arena(100);  // reserve the arena size

    //tree.clear();
    ryml::NodeRef root = tree.rootref();
    //root |= ryml::MAP; // mark root as a map
    ToYml(&root, value_);

    tree.rootref() = root;

    content = ryml::emitrs_yaml<std::string>(tree);
    auto contentChar = content.c_str();
    return String(contentChar);
#else
    return "";
#endif
}

bool YAMLFile::Save(Serializer& dest) const 
{
    SE_LOG_WARNING("YAMLFile::ToString() implementation");
    return dest.WriteString(ToString());
}



}