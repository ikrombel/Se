#include <SeResource/YAMLFile.h>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml-0.7.2/rapidyaml.hpp"

#include <Se/Console.hpp>
#include <Se/String.hpp>
#include <Se/IO/Serializer.hpp>
#include <Se/IO/Deserializer.hpp>
#include <SeResource/JSONValue.h>

#include <cstring>

#ifndef strtof64
#define strtof64 strtof
#endif

namespace Se
{

Value ToValue(const ryml::ConstNodeRef& node)
{
    Value value;
    if (node.is_map())
    {
        for (auto child : node.children())
        {
            auto keyTmp = String(child.key().str, child.key().size());
            value.Set(keyTmp, ToValue(child));
        }
    }
    else if (node.is_seq())
    {
        for (auto child : node.children())
        {
            value.Push(ToValue(child));
        }
    }
    else if(node.key_is_null() && node.val_is_null())
    {

    }
    else if (node.is_keyval())
    {  
        auto yamlValue = node.val();
        Value tmpValue = Value::EMPTY;

        // Handle primitive types
        if (yamlValue.is_number() && yamlValue.is_integer()) 
        {
            long long llValue = strtoll(yamlValue.str, nullptr, 10);
            if (llValue >= INT_MIN && llValue <= INT_MAX)
                tmpValue = (int)llValue;
            else
                tmpValue = String(yamlValue.str, yamlValue.size());
        }
        else if (yamlValue.is_number() && yamlValue.is_unsigned_integer())
        {
            unsigned long long ullValue = strtoull(yamlValue.str, nullptr, 10);
            if (ullValue >= 0 && ullValue <= UINT_MAX)
                tmpValue = (unsigned)ullValue;
            else
                tmpValue = String(yamlValue.str, yamlValue.size());
        }
        else if (yamlValue.is_number() && yamlValue.is_real()) {
            tmpValue = yamlValue.str ? strtof64(yamlValue.str, nullptr) : 0;
        } 
        else {
            String tmpStr = yamlValue.str ? String(yamlValue.str, yamlValue.size()) : "";
            if (tmpStr == "true" || tmpStr == "false")
                tmpValue = ToBool(tmpStr);

            else if (yamlValue.str)
                tmpValue = yamlValue.str ? String(yamlValue.str, yamlValue.size()) : "";
        }

        value = tmpValue;
    }

    else if (node.has_val())
    {
        value = Value::EMPTY;
    }
    return value;
}

void ToYml(ryml::NodeRef* node, const Value& input)
{
    if (input.IsObject())
    {
        *node |= ryml::MAP;
        //*node |= ryml::FLOW_SL; // flow, single-line

        for (auto& [key, value] : input.GetObject())
        {
            ryml::NodeRef child = node->append_child();
            child.set_key(ryml::csubstr(key.c_str()));
            ToYml(&child, value);
            (*node)[key.c_str()] = child;
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
        node->set_val(std::move(input.GetString().c_str()));
    }
    else if (input.IsNumber() && input.GetNumberType() == ValueNumberType::VALUE_NT_INT)
    {
        node->set_val(cformat("%i", input.GetInt()).c_str());
    }
    else if (input.IsNumber() && input.GetNumberType() == ValueNumberType::VALUE_NT_FLOAT_DOUBLE)
    {
        //auto strDouble = std::to_string((double)input.GetDouble());
        auto strDouble = cformat("%.9f", input.GetDouble());
        node->set_val(std::move(strDouble).c_str());
    }
    // child |= ryml::FLOW_SL; // flow, single-line
    else if (input.IsBool())
    {
        node->set_val(input.GetBool() ? "true" : "false");
    }
    // else
    //     node->set_val(input.To.c_str());
}

bool YAMLFile::BeginLoad(Deserializer& source) {
    
    String content = source.ReadString();
    ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(content));
    value_ = ToValue(tree.crootref());
    return true;
}

String YAMLFile::ToString() const
{
    std::string content;
    content.reserve(10000);

    ryml::Tree tree;

    tree.reserve(10);         // reserve the number of nodes
    tree.reserve_arena(100);  // reserve the arena size

    //tree.clear();
    ryml::NodeRef root = tree.rootref();
    //root |= ryml::MAP; // mark root as a map
    ToYml(&root, value_);

    //tree.rootref() = root;

    content = ryml::emitrs_yaml<std::string>(tree);
    return String(content.c_str(), content.size());
}

bool YAMLFile::Save(Serializer& dest) const 
{
    return dest.WriteString(ToString());
}

}