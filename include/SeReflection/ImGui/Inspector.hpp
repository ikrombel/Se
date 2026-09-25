#pragma once
#include <SeReflection/Reflected.hpp>
#include <SeReflection/ImGui/ImScopeGuard.hpp>
//#include <SeReflection/ImGui/Inspector.SeMath.hpp>

#include <Se/Format.hpp>
#include <Se/String.hpp>


#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <cmath>

#include <SeReflection/ImGui/Widgets.hpp>

namespace Se
{

struct Inspector
{
    struct Item {
        String name_;
        String hint_;
        EditOptions options_;
    };

    template<class T>
    void Link(T* object) {
        reflected_ =  Reflected<T>::ToReflectedObject(object);
    }

    void Render(const String& name = String::EMPTY);

    template<typename T, typename = void>
    struct HasRegisterRenderImGui : std::false_type {};

    template<typename T>
    struct HasRegisterRenderImGui<T,
        std::void_t<decltype(std::declval<T>().RenderImGui(std::declval<const char*>()))>>
        : std::true_type {};

    std::vector<Item> Get(const String& typeName) {
        return registered_[typeName];
    }

    using RenderFunc = std::function<bool(const char*, AttributePtr&, const EditOptions&)>;

    ///TODO Simprify
    template <typename... Args>
    void Register(bool reset = false);
    //void Register(const std::initializer_list<std::pair<std::size_t, RenderFunc>>& init, bool reset = false);

    std::optional<Item> GetPropertyDesc(const std::string& typeName, const String& propertyName) {
        auto it = registered_.find(typeName);

        if (it == registered_.end())
            return std::nullopt;

        //std::unordered_map<String, PropertyDesc>& objIter = it->second;
        //auto itPar = FindDesc(it->second, propertyName);
        //itPar = it->second.find(propertyName);

        auto pName = propertyName;
        auto itPar = std::find_if(it->second.begin(), it->second.end(), [pName](Item pDesc) {
                return pDesc.name_ == pName;
            });

        if (itPar == it->second.end())
            return std::nullopt;

        return *itPar;
    }

    bool ParameterSettings(const String& typeName, const std::vector<Item>& descs) {
        auto it = registered_.find(typeName);
        if (it != registered_.end())
            return false;

        registered_.insert_or_assign(typeName, descs);

        return true;
    }

    void Unregister(const String& typeName)
    {
        auto it = registered_.find(typeName);

        if (it != registered_.end())
            registered_.erase(it);
    }
private:
    std::shared_ptr<ReflectedObject> reflected_;
    std::unordered_map<std::string, std::vector<Item>> registered_;

   
    std::unordered_map<std::size_t, RenderFunc> RenderRegisterImGui;

};


template<class T>
inline bool RenderParameter(const char* label, T& value, const EditOptions& opt)
{
    if constexpr (Inspector::HasRegisterRenderImGui<T>::value) {
        return value.RenderImGui(label);
    }

    //TODO Add cheking method: RenderImGui(const char* label)
    SE_LOG_TODO("Do not implemented for type: {}\n", ToStringTypeId<T>());
    return false;
}


template<typename T>
inline bool WrapParameter(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<T>();

    T value{};
    attrAccesor->Get(&value);

    if (RenderParameter<T>(label, value, options))
        attrAccesor->Set(value);

    return false;
}


struct RenderTODO{};

bool RenderAttributeTODO(const char* label, AttributePtr& attr, const EditOptions& opt = {});


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
inline void Inspector::Render(const String& name)
{
    if (!reflected_)
        return;

    auto idHeader = format("{}###{:X}.header",
        name.size() ? name : reflected_->GetStaticType(),
        static_cast<int>(*(reflected_->Cast<uintptr_t>()))
        );

    ImGui::PushID(reflected_.get());
    bool collapsed = ImGui::CollapsingHeader(idHeader.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopID();

    if (!collapsed || !ImGui::BeginTable("RenderObject::Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
        return;

    auto attributeNames = reflected_->GetAttriburesNames();

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
        auto attr = reflected_->FindAttribute(attrName);

        int p = static_cast<int>(reinterpret_cast<uintptr_t>(attr.get()));
        auto id = format("###{:X}.{}", p, attrName);
        auto idLabel = id + ".label";
        auto idPopup = id + ".popup";


        auto decr = Inspector::GetPropertyDesc(reflected_->GetType(), attrName);
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

        auto render = RenderRegisterImGui.find(attr->GetValueType());

        if (attr->GetType() == AttributeType::AT_Enum)
        {
            auto enumNames = attr->GetEnumNames();

            auto attrAccesor = reinterpret_cast<AttributeAccessor<int>*>(attr.get());
            int value{};
            attrAccesor->Get(&value);

            if (value >= enumNames.size())
                value = 0;

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (Widget::EditEnum(id.c_str(), value, attr->GetEnumNames())) {
                 attrAccesor->Set(value);
            }
        }
        else if (render != RenderRegisterImGui.end())
            render->second(id.c_str(), attr, options);
        else
        {
            RenderAttributeTODO(id.c_str(), attr);
        }



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

inline bool RenderAttributeTODO(const char* label, AttributePtr& attr, const EditOptions& opt)
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
                "template<> bool RenderParameter<%s>(const char* label, %s& attr, const EditOptions& opt) {\n"
                "  //implement by ImGui)\n"
                "}\n"
                "} // namespace Se\n",
                typeName.c_str(), typeName.c_str());
        ImGui::EndTooltip();
    }

    return false;
}


inline bool RenderParameter(const char* label, int& value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ : ImGui::GetContentRegionAvail().x);
    return ImGui::DragInt(label, &value, std::max(1.0, options.step_), options.min_, options.max_);    
}

template<>
inline bool RenderParameter(const char* label, float& value, const EditOptions& options)
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

    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    return ImGui::DragFloat(label, &value, options.step_, options.min_, options.max_,
                GetFormatStringForStep(options.step_).c_str());
}

#if 1
template<>
bool RenderParameter(const char* label, String& attr, const EditOptions& options);
#else
template<>
inline bool RenderParameter(const char* label, String& value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    return ImGui::InputText(label, (std::string*)&value, ImGuiInputTextFlags_EnterReturnsTrue);
}
#endif

template <typename... Args>
inline void Inspector::Register(bool reset)
{
    if (reset)
    {
        RenderRegisterImGui.clear();
        Inspector::Register<int, float, bool, long long, Se::String>();
    }

    // c++17 features
    RenderRegisterImGui.insert({
        { typeid(Args).hash_code(), &WrapParameter<Args> }...
    });
}

} // namespace Se
