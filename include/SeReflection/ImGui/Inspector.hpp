#pragma once
#include <SeReflection/Reflected.hpp>
#include <SeReflection/ImGui/ImScopeGuard.hpp>
//#include <SeReflection/ImGui/Inspector.SeMath.hpp>

#include <Se/Format.hpp>
#include <Se/String.hpp>


#include <optional>
#include <unordered_map>
#include <cmath>

#include <imgui.h>

namespace Se
{

template<typename T, typename = void>
struct HasRegisterRenderImGui : std::false_type {};

template<typename T>
struct HasRegisterRenderImGui<T,
    std::void_t<decltype(std::declval<T>().RenderImGui(std::declval<const char*>()))>>
    : std::true_type {};

struct EditOptions
{
    /// Increment per pixel for scalar scrolls.
    double step_{0.01};
    /// Minimum value (for component).
    double min_{0.0f};
    /// Maximum value (for component).
    double max_{0.0f};
    /// Whether to treat Vector3 and Vector4 as color values.
    bool asColor_{};
    /// Whether to allow resize for dynamically sized containers.
    bool allowResize_{};
    /// Whether to allow element type changes for containers.
    bool allowTypeChange_{};
    /// Whether to treat integer as bitmask.
    bool asBitmask_{};
    /// Whether to extract elements metadata dynamically from the inspected StringVariantMap itself.
    bool dynamicMetadata_{};
    /// Enum values used to convert integer to string.
    const StringVector* intToString_{};
    /// Allowed resource types.
    const std::vector<String>* resourceTypes_{};
    /// Structure array element names.
    const StringVector* sizedStructVectorElements_{};
    const bool* disabled_{};
    bool hidden_{false};

    std::optional<float> componentWidth_{std::nullopt};

    // EditOptions& operator=(const EditOptions& rhs)
    // {
    //     step_ = rhs.step_;
    //     min_ = rhs.min_;
    //     max_ = rhs.max_;
    //     asColor_ = rhs.asColor_;
    //     allowResize_ = rhs.allowResize_;
    //     allowTypeChange_ = rhs.allowTypeChange_;
    //     asBitmask_ = rhs.asBitmask_;
    //     dynamicMetadata_ = rhs.dynamicMetadata_;
    //     intToString_ = rhs.intToString_;

    //     return *this;
    // }

    bool IsDisabled(bool defaultValue) {
        return disabled_ ? *disabled_ : defaultValue; }

    EditOptions& AsColor() { asColor_ = true; return *this; }
    EditOptions& AsBitmask() { asBitmask_ = true; return *this; }
    EditOptions& Range(double min, double max) { min_ = min; max_ = max; return *this; }
    EditOptions& Step(double step) { step_ = step; return *this; }
    EditOptions& Enum(const StringVector& values) { intToString_ = &values; return *this; }
    EditOptions& ResourceTypes(const StringVector& types) { resourceTypes_ = &types; return *this; }
    EditOptions& SizedStructVector(const StringVector& names) { sizedStructVectorElements_ = &names; return *this; }
    EditOptions& AllowResize() { allowResize_ = true; return *this; }
    EditOptions& AllowTypeChange() { allowTypeChange_ = true; return *this; }
    EditOptions& ComponentWidth(float width) { componentWidth_ = width; return *this; }
    EditOptions& DynamicMetadata() { dynamicMetadata_ = true; return *this; }
    EditOptions& Disabled(bool* value) { disabled_ = value; return *this; }
    EditOptions& Hidden() { hidden_ = true; return *this; }
};

struct PropertyDesc
{
    String name_;
    String hint_;
    EditOptions options_;

    // PropertyDesc& operator=(const PropertyDesc& rhs)
    // {
    //     name_ = rhs.name_;
    //     hint_ = rhs.hint_;
    //     options_ = rhs.options_;

    //     return *this;
    // }

    // bool operator==(const PropertyDesc& rhs) const
    // {
    //     return hint_ == rhs.hint_;
    // }

    // bool operator bool() const
    // {
    //     return hint_ == rhs.hint_;
    // }


    static std::vector<PropertyDesc> Get(const String& typeName) {
        return registered_[typeName];
    }

    static std::optional<PropertyDesc> GetPropertyDesc(const String& typeName, const String& propertyName) {
        auto it = registered_.find(typeName);

        if (it == registered_.end())
            return std::nullopt;

        //std::unordered_map<String, PropertyDesc>& objIter = it->second;
        //auto itPar = FindDesc(it->second, propertyName);
        //itPar = it->second.find(propertyName);

        auto pName = propertyName;
        auto itPar = std::find_if(it->second.begin(), it->second.end(), [pName](PropertyDesc pDesc) {
                return pDesc.name_ == pName;
            });

        if (itPar == it->second.end())
            return std::nullopt;

        return *itPar;
    }

    static bool Register(const String& typeName, const std::vector<PropertyDesc>& descs) {
        auto it = registered_.find(typeName);
        if (it != registered_.end())
            return false;

        registered_.insert_or_assign(typeName, descs);

        return true;
    }

    static void Unregister(const String& typeName)
    {
        auto it = registered_.find(typeName);

        if (it != registered_.end())
            registered_.erase(it);
    }
private:
    static std::unordered_map<String, std::vector<PropertyDesc>> registered_;

};

inline std::unordered_map<String, std::vector<PropertyDesc>> PropertyDesc::registered_;


namespace Widgets
{

inline bool EditEnum(const char* label, int& var, const Se::StringVector& items)
{
    //const auto& items = *options.intToString_;
    const auto maxEnumValue = static_cast<int>(items.size() - 1);
    bool valueChanged = false;

    int value = std::clamp(var, 0, maxEnumValue);
    //ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("", items[value].c_str()))
    {
        for (int index = 0; index <= maxEnumValue; ++index)
        {
            if (ImGui::Selectable(items[index].c_str(), value == index))
            {
                var = index;
                valueChanged = true;
                break;
            }
        }
        ImGui::EndCombo();
    }
    return valueChanged;
}

} // namespace Widgets

template<class T>
inline bool Render(const char* label, AttributePtr& attr, const EditOptions& opt = {})
{
    auto attrAccessor = attr->AccesorCast<T>();

    if constexpr (HasRegisterRenderImGui<T>::value) {
        T* value = attrAccessor->GetPtr();
        if (value)
            return value->RenderImGui(label);
    }

    //TODO Add cheking method: RenderImGui(const char* label)
    SE_LOG_TODO("Do not implemented for type: {}\n", ToStringTypeId<T>());
    return false;
}

class RenderObjectScope
{
    friend void RenderObject(Se::ReflectedObject* obj, const String& name);
public:
    using Func = std::function<bool(const char*, AttributePtr&, const EditOptions&)>;

    template<typename... Ts>
    static RenderObjectScope Register()
    {
        // String debugStr = "Register:";
        //  ((debugStr += format(" {}", ToStringTypeId<Ts>())), ...);
        // SE_LOG_DEBUG(debugStr);
        return RenderObjectScope{ { { typeid(Ts).hash_code(), &Render<Ts> }... } };
    }

    virtual ~RenderObjectScope()
    {
        String debugStr = "Unregister:";
        for(auto hashName : hashNames_)
        {
            auto it = RenderRegisterImGui.find(hashName);
            if (it != RenderRegisterImGui.end())
                RenderRegisterImGui.erase(it);
            debugStr += format(" {:X}", hashName);
        }
        SE_LOG_DEBUG(debugStr);
    }
private:
    //RenderObjectScope(std::size_t hash, Func func);
    RenderObjectScope(const std::initializer_list<std::pair<std::size_t, Func>>& init)
    {
        for (auto it : init)
        {
            hashNames_.push_back(it.first);
            RenderRegisterImGui[it.first] = it.second;
        }
    }

//    std::size_t hashName_;
    std::vector<std::size_t> hashNames_;
    static std::unordered_map<std::size_t, Func> RenderRegisterImGui;
};





struct RenderTODO{};

template<>
bool Render<RenderTODO>(const char* label, AttributePtr& attr, const EditOptions& opt);


struct RenderObjectNode {
    RenderObjectNode* node;
    std::shared_ptr<AttributeEmpty> attribute;
};

struct RenderObjectSettings
{
    int Type; // View type as Header / Tab / Menu
    String Title;
    RenderObjectNode node;
};

//!TODO inline void RenderObject(Se::ReflectedObject* obj, RenderObjectSettings& settings)
inline void RenderObject(Se::ReflectedObject* obj, const String& name = String::EMPTY)
{
    auto idHeader = format("{}###{:X}.header",
        name.size() ? name : obj->GetStaticType(),
        static_cast<int>(*obj->Cast<uintptr_t>())
        );

    ImGui::PushID(obj);
    bool collapsed = ImGui::CollapsingHeader(idHeader.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopID();

    if (!collapsed || !ImGui::BeginTable("RenderObject::Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
        return;

    auto attributeNames = obj->GetAttriburesNames();

    // { // Sort attributed with AttributeType::AT_Action to bottom
    //     //auto objRef = reinterpret_cast<Se::ReflectedObject*>(obj);
    //     auto objRef = obj; //->Cast<Se::ReflectedObject>();
    //     std::sort(attributeNames.begin(), attributeNames.end(), [objRef](const String& lhs, const String& rhs) {
    //         auto attr = objRef->FindAttribute(rhs);
    //         return attr->GetType() == AttributeType::AT_Action;
    //     });
    // }


    for (auto& attrName : attributeNames)
    {
        auto attr = obj->FindAttribute(attrName);

        int p = static_cast<int>(reinterpret_cast<uintptr_t>(attr.get()));
        auto id = format("###{:X}.{}", p, attrName);
        auto idLabel = id + ".label";
        auto idPopup = id + ".popup";


        auto decr = PropertyDesc::GetPropertyDesc(obj->GetType(), attrName);
        auto options = decr ? decr.value().options_ : EditOptions{};

        bool disabled = false;

        if (decr)
        {
            disabled = options.IsDisabled(false);

            if (options.hidden_)
                continue;
        }

        ImGui::TableNextRow();

        ImGuiDisabledGuard disabledGuard(disabled);

        if (attr->GetType() == AttributeType::AT_Action)
        {
            ImGui::TableSetColumnIndex(1);

            // ColorScopeGuard guard({
            //       { ImGuiCol_Button, ImVec4(0.7f, 0.4f, 0.1f, 1.0f)}
            //     //, { ImGuiCol_Text, Color::BLACK }
            // });
            //ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Button(attrName, {ImGui::GetContentRegionAvail().x, 25}))
                attr->Call();

            continue;
        }
        else
        {
            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            //ImGui::PushID(id.c_str());


            ImGui::LabelText(idLabel.c_str(), "%s", attrName.c_str());

            // if (ImGui::IsItemHovered())
            //     ImGui::SetTooltip("%s %lx", attrName.c_str(), attr->GetValueType());


            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup(idPopup.c_str());
            }

        }

        ImGui::TableSetColumnIndex(1);

        auto render = RenderObjectScope::RenderRegisterImGui.find(attr->GetValueType());

        if (attr->GetType() == AttributeType::AT_Enum)
        {
            auto enumNames = attr->GetEnumNames();

            auto attrAccesor = reinterpret_cast<AttributeAccessor<int>*>(attr.get());
            int value{};
            attrAccesor->Get(&value);

            if (value >= enumNames.size())
                value = 0;

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (Widgets::EditEnum(id.c_str(), value, attr->GetEnumNames())) {
                 attrAccesor->Set(value);
            }
        }
        else if (render != RenderObjectScope::RenderRegisterImGui.end())
            render->second(id.c_str(), attr, options);
        else
            Render<RenderTODO>(id.c_str(), attr);



        // This block must be called every frame, even when the popup is closed
        if (ImGui::BeginPopup(idPopup.c_str())) {
            ImGui::TextDisabled("%s.%s\ntypeId: %lx", name.c_str(), attrName.c_str(), attr->GetValueType());
            if (decr)
            {
                ImGui::TextDisabled("%s", decr.value().hint_.c_str());
            }

            ImGui::Separator();
            if (ImGui::Selectable("To default")) {
                attr->ToDefault();
            }
            // if (ImGui::Selectable("Option 2")) {
            //     // Handle Option 2 action
            // }
            ImGui::EndPopup();
        }
    }

    ImGui::EndTable();
}

template<>
inline bool Render<RenderTODO>(const char* label, AttributePtr& attr, const EditOptions& opt)
{
    //Se::ColorScopeGuard guard (ImGuiSty)
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "TODO:NotImplemented");

    ImGui::SameLine();
    //ImGui::Button("copy");

    if (ImGui::IsItemHovered()) {
        auto typeName = attr->GetTypeName();
        ImGui::BeginTooltip();
            ImGui::TextWrapped("type: %s, need implement \n", typeName.c_str());

            ImGui::Text(
                "namespace Se {\n"
                "template<> bool Render<%s>(const char* label, AttributePtr& attr, const EditOptions& opt) {\n"
                "  //implement by ImGui)\n"
                "}\n"
                "} // namespace Se\n",
                typeName.c_str());
        ImGui::EndTooltip();
    }

    return false;
}

template<>
inline bool Render<int>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<int>();

    int value{0};
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ : ImGui::GetContentRegionAvail().x);
    if (ImGui::DragInt(label, &value, std::max(1.0, options.step_), options.min_, options.max_))
        attrAccesor->Set(value);

    return false;
}



template<>
inline bool Render<float>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto GetFormatStringForStep = [](double step) -> String
    {
        if (step >= 1.0 || step <= 0.0)
            return "%.0f";
        else {
            const auto numDigits = std::clamp(static_cast<int>(std::round(-std::log10(step))), 1, 8);
            return format("%.{}f", numDigits);
        }
    };


    auto attrAccesor = attr->AccesorCast<float>();

    float value{0.f};
    attrAccesor->Get(&value);

    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat(label, &value, options.step_, options.min_, options.max_,
                      GetFormatStringForStep(options.step_).c_str()))
    {
        attrAccesor->Set(value);
        return true;
    }

    return false;
}

template<>
inline bool Render<bool>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<bool>();

    bool value{0};
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::Checkbox(label, &value)) {
        attrAccesor->Set(value);
        return true;
    }

    return false;
}

template<>
inline bool Render<ImVec2>(const char* label, AttributePtr& attr, const EditOptions& opt)
{
    auto attrAccesor = attr->AccesorCast<ImVec2>();

    ImVec2 value{};
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat2(label, &value.x))
        attrAccesor->Set(value);

    return false;
}

template<>
inline bool Render<ImVec4>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<ImVec4>();

    //TODO add support Localization
    String title = label;

    ImVec4 value{};
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (options.asColor_) {
       if (ImGui::ColorEdit4(title.c_str(), &value.x))
        attrAccesor->Set(value);
    }
    else if (ImGui::DragFloat4(title.c_str(), &value.x))
        attrAccesor->Set(value);
    return false;
}

#if 1
template<>
bool Render<String>(const char* label, AttributePtr& attr, const EditOptions& options);
#else
template<>
inline bool Render<String>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<String>();

    String value;
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText(label, (std::string*)&value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        attrAccesor->Set(value);
        return true;
    }

    return false;
}
#endif

inline std::unordered_map<std::size_t, RenderObjectScope::Func> RenderObjectScope::RenderRegisterImGui = {
      {typeid(int).hash_code(), &Render<int>}
    , {typeid(float).hash_code(), &Render<float>}
    , {typeid(bool).hash_code(), &Render<bool>}
    , {typeid(long long).hash_code(), &Render<long long>}
    , {typeid(Se::String).hash_code(), &Render<String>}
};

} // namespace Se
