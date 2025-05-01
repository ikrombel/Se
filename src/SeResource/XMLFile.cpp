
#include <Se/Profiler.hpp>
#include <Se/IO/Deserializer.hpp>
#include <Se/Console.hpp>
#include <Se/IO/MemoryBuffer.hpp>
#include <Se/IO/VectorBuffer.h>
#include <SeVFS/VirtualFileSystem.h>
//#include <Se/IO/VectorBuffer.h>
//#include <GFrost/Resource/ResourceCache.h>
//#include <SeResource/XMLArchive.h>

#include <SeResource/XMLFile.h>
//#include <SeResource/XMLArchive.h>

#include "PugiXml/pugixml.hpp"

#include <vector>


namespace Se
{

/// XML writer for pugixml.
class XMLWriter : public pugi::xml_writer
{
public:
    /// Construct.
    explicit XMLWriter(Serializer& dest) :
        dest_(dest),
        success_(true)
    {
    }

    /// Write bytes to output.
    void write(const void* data, std::size_t size) override
    {
        if (dest_.Write(data, (unsigned)size) != size)
            success_ = false;
    }

    /// Destination serializer.
    Serializer& dest_;
    /// Success flag.
    bool success_;
};

XMLFile::XMLFile() :
    Resource("XMLFile"),
    document_(new pugi::xml_document())
{
}

XMLFile::~XMLFile() = default;


bool XMLFile::BeginLoad(Deserializer& source)
{
    unsigned dataSize = source.GetSize();
    if (!dataSize && !source.GetName().empty())
    {
        SE_LOG_ERROR("Zero sized XML data in " + source.GetName());
        return false;
    }

    std::shared_ptr<char> buffer(new char[dataSize], std::default_delete<char[]>());
    if (source.Read(buffer.get(), dataSize) != dataSize)
        return false;

    if (!document_->load_buffer(buffer.get(), dataSize))
    {
        SE_LOG_ERROR("Could not parse XML data from " + source.GetName());
        document_->reset();
        return false;
    }

    XMLElement rootElem = GetRoot();
    String inherit = rootElem.GetAttribute("inherit");
    if (!inherit.empty())
    {
        // The existence of this attribute indicates this is an RFC 5261 patch file
        //auto* cache = GetSubsystem<ResourceCache>();
        //auto vfs = VirtualFileSystem::Get();
        // If being async loaded, GetResource() is not safe, so use GetTempResource() instead
        auto inheritedXMLFile = std::make_shared<XMLFile>();
        inheritedXMLFile->LoadFile(inherit);
        //XMLFile* inheritedXMLFile = GetAsyncLoadState() == ASYNC_DONE ? cache->GetResource<XMLFile>(inherit) :
        //    cache->GetTempResource<XMLFile>(inherit);
        if (!inheritedXMLFile)
        {
            SE_LOG_ERROR("Could not find inherited XML file: {}", inherit.c_str());
            return false;
        }

        // Patch this XMLFile and leave the original inherited XMLFile as it is
        std::unique_ptr<pugi::xml_document> patchDocument(document_.release());
        document_ = std::make_unique<pugi::xml_document>();
        document_->reset(*inheritedXMLFile->document_);
        Patch(rootElem);

        // Store resource dependencies so we know when to reload/repatch when the inherited resource changes
        //cache->StoreResourceDependency(this, inherit);

        // Approximate patched data size
        dataSize += inheritedXMLFile->GetMemoryUse();
    }

    // Note: this probably does not reflect internal data structure size accurately
    SetMemoryUse(dataSize);
    return true;
}

bool XMLFile::Save(Serializer& dest) const
{
    return Save(dest, "\t");
}

bool XMLFile::Save(Serializer& dest, const String& indentation) const
{
    XMLWriter writer(dest);
    document_->save(writer, indentation.c_str());
    return writer.success_;
}

// bool XMLFile::SaveObjectCallback(const std::function<void(Archive&)> serializeValue)
// {
//     try
//     {
//         document_->remove_attributes();
//         document_->remove_children();
//         XMLOutputArchive archive{this};
//         serializeValue(archive);
//         return true;
//     }
//     catch (const ArchiveException& e)
//     {
//         document_->remove_attributes();
//         document_->remove_children();
//         SE_LOG_ERROR("Failed to save object to XML: {}", e.what());
//         return false;
//     }
// }

// bool XMLFile::LoadObjectCallback(const std::function<void(Archive&)> serializeValue) const
// {
//     try
//     {
//         XMLInputArchive archive((XMLFile*)this);
//         serializeValue(archive);
//         return true;
//     }
//     catch (const ArchiveException& e)
//     {
//         SE_LOG_ERROR("Failed to load object from XML: {}", e.what());
//         return false;
//     }
// }
    
XMLElement XMLFile::CreateRoot(const String& name)
{
    document_->reset();
    pugi::xml_node root = document_->append_child(name.c_str());
    return XMLElement(this, root.internal_object());
}

XMLElement XMLFile::GetOrCreateRoot(const String& name)
{
    XMLElement root = GetRoot(name);
    if (root.NotNull())
        return root;
    root = GetRoot();
    if (root.NotNull())
        SE_LOG_WARNING("XMLFile already has root {}, deleting it and creating root {}", root.GetName(), name);
    return CreateRoot(name);
}

bool XMLFile::FromString(const String& source)
{
    if (source.empty())
        return false;

    MemoryBuffer buffer(source.c_str(), source.length());
    return Load(buffer);
}

XMLElement XMLFile::GetRoot(const String& name)
{
    pugi::xml_node root = document_->first_child();
    if (root.empty())
        return XMLElement();

    if (!name.empty() && name != root.name())
        return XMLElement();
    else
        return XMLElement(this, root.internal_object());
}

String XMLFile::ToString(const String& indentation) const
{
    VectorBuffer dest;
    XMLWriter writer(dest);
    document_->save(writer, indentation.c_str());
    return String((const char*)dest.GetData(), dest.GetSize());
}

void XMLFile::Patch(XMLFile* patchFile)
{
    Patch(patchFile->GetRoot());
}

void XMLFile::Patch(const XMLElement& patchElement)
{
    pugi::xml_node root = pugi::xml_node(patchElement.GetNode());

    for (auto& patch : root)
    {
        pugi::xml_attribute sel = patch.attribute("sel");
        if (sel.empty())
        {
            SE_LOG_ERROR("XML Patch failed due to node not having a sel attribute.");
            continue;
        }

        // Only select a single node at a time, they can use xpath to select specific ones in multiple otherwise the node set becomes invalid due to changes
        //pugi::xpath_node original = document_->select_single_node(sel.value());
        pugi::xpath_node original = document_->select_node(sel.value());
        if (!original)
        {
            SE_LOG_ERROR("XML Patch failed with bad select: {}.", sel.value());
            continue;
        }

        if (strcmp(patch.name(), "add") == 0)
            PatchAdd(patch, original);
        else if (strcmp(patch.name(), "replace") == 0)
            PatchReplace(patch, original);
        else if (strcmp(patch.name(), "remove") == 0)
            PatchRemove(original);
        else
            SE_LOG_ERROR("XMLFiles used for patching should only use 'add', 'replace' or 'remove' elements.");
    }
}

void XMLFile::PatchAdd(const pugi::xml_node& patch, pugi::xpath_node& original) const
{
    // If not a node, log an error
    if (original.attribute())
    {
        SE_LOG_ERROR("XML Patch failed calling Add due to not selecting a node, {} attribute was selected.",
            original.attribute().name());
        return;
    }

    // If no type add node, if contains '@' treat as attribute
    pugi::xml_attribute type = patch.attribute("type");
    if (!type || strlen(type.value()) <= 0)
        AddNode(patch, original);
    else if (type.value()[0] == '@')
        AddAttribute(patch, original);
}

void XMLFile::PatchReplace(const pugi::xml_node& patch, pugi::xpath_node& original) const
{
    // If no attribute but node then its a node, otherwise its an attribute or null
    if (!original.attribute() && original.node())
    {
        pugi::xml_node parent = original.node().parent();

        parent.insert_copy_before(patch.first_child(), original.node());
        parent.remove_child(original.node());
    }
    else if (original.attribute())
    {
        original.attribute().set_value(patch.child_value());
    }
}

void XMLFile::PatchRemove(const pugi::xpath_node& original) const
{
    // If no attribute but node then its a node, otherwise its an attribute or null
    if (!original.attribute() && original.node())
    {
        pugi::xml_node parent = original.parent();
        parent.remove_child(original.node());
    }
    else if (original.attribute())
    {
        pugi::xml_node parent = original.parent();
        parent.remove_attribute(original.attribute());
    }
}

void XMLFile::AddNode(const pugi::xml_node& patch, const pugi::xpath_node& original) const
{
    // If pos is null, append or prepend add as a child, otherwise add before or after, the default is to append as a child
    pugi::xml_attribute pos = patch.attribute("pos");
    if (!pos || strlen(pos.value()) <= 0 || strcmp(pos.value(), "append") == 0)
    {
        pugi::xml_node::iterator start = patch.begin();
        pugi::xml_node::iterator end = patch.end();

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the first node of the nodes to add
        if (CombineText(patch.first_child(), original.node().last_child(), false))
            start++;

        for (; start != end; start++)
            original.node().append_copy(*start);
    }
    else if (strcmp(pos.value(), "prepend") == 0)
    {
        pugi::xml_node::iterator start = patch.begin();
        pugi::xml_node::iterator end = patch.end();

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the last node of the nodes to add
        if (CombineText(patch.last_child(), original.node().first_child(), true))
            end--;

        pugi::xml_node pos = original.node().first_child();
        for (; start != end; start++)
            original.node().insert_copy_before(*start, pos);
    }
    else if (strcmp(pos.value(), "before") == 0)
    {
        pugi::xml_node::iterator start = patch.begin();
        pugi::xml_node::iterator end = patch.end();

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the first node of the nodes to add
        if (CombineText(patch.first_child(), original.node().previous_sibling(), false))
            start++;

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the last node of the nodes to add
        if (CombineText(patch.last_child(), original.node(), true))
            end--;

        for (; start != end; start++)
            original.parent().insert_copy_before(*start, original.node());
    }
    else if (strcmp(pos.value(), "after") == 0)
    {
        pugi::xml_node::iterator start = patch.begin();
        pugi::xml_node::iterator end = patch.end();

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the first node of the nodes to add
        if (CombineText(patch.first_child(), original.node(), false))
            start++;

        // There can not be two consecutive text nodes, so check to see if they need to be combined
        // If they have been we can skip the last node of the nodes to add
        if (CombineText(patch.last_child(), original.node().next_sibling(), true))
            end--;

        pugi::xml_node pos = original.node();
        for (; start != end; start++)
            pos = original.parent().insert_copy_after(*start, pos);
    }
}

void XMLFile::AddAttribute(const pugi::xml_node& patch, const pugi::xpath_node& original) const
{
    pugi::xml_attribute attribute = patch.attribute("type");

    if (!patch.first_child() && patch.first_child().type() != pugi::node_pcdata)
    {
        SE_LOG_ERROR("XML Patch failed calling Add due to attempting to add non text to an attribute for {}.", attribute.value());
        return;
    }

    String name(attribute.value());
    name = name.substr(1);

    pugi::xml_attribute newAttribute = original.node().append_attribute(name.c_str());
    newAttribute.set_value(patch.child_value());
}

bool XMLFile::CombineText(const pugi::xml_node& patch, const pugi::xml_node& original, bool prepend) const
{
    if (!patch || !original)
        return false;

    if ((patch.type() == pugi::node_pcdata && original.type() == pugi::node_pcdata) ||
        (patch.type() == pugi::node_cdata && original.type() == pugi::node_cdata))
    {
        if (prepend)
            const_cast<pugi::xml_node&>(original).set_value(cformat("%s%s", patch.value(), original.value()).c_str());
        else
            const_cast<pugi::xml_node&>(original).set_value(cformat("%s%s", original.value(), patch.value()).c_str());

        return true;
    }

    return false;
}

}
