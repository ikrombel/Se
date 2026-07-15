#pragma once

#include <Se/String.hpp>
#include <SeReflection/ImGui/Inspector.hpp>

#include <SeMath/Vector4.hpp>
#include <SeMath/Color.hpp>
#include <SeMath/Rect.hpp>

#include <IconsFontAwesome5.h>


#include <imgui_internal.h>


namespace ImGui
{

bool InputText(const char* label, Se::String* str, ImGuiInputTextFlags flags = 0,
    ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

namespace Private
{

struct InputTextCallback_UserData
{
    Se::String* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

} // namespace :Private

inline bool InputText(const char* label, Se::String* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    auto InputTextCallback = [](ImGuiInputTextCallbackData* data) -> int
    {
        auto* user_data = (Private::InputTextCallback_UserData*)data->UserData;
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            Se::String* str = user_data->Str;
            IM_ASSERT(data->Buf == str->c_str());
            str->resize(data->BufTextLen);
            data->Buf = (char*)str->c_str();
        }
        else if (user_data->ChainCallback)
        {
            // Forward to user callback, if any
            data->UserData = user_data->ChainCallbackUserData;
            return user_data->ChainCallback(data);
        }
        return 0;
    };


    //IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    Private::InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

} // namespace ImGui

namespace Se
{


inline ImVec2 ToImGui(const Vector2& value) { return {value.x_, value.y_}; };
inline ImVec2 ToImGui(const IntVector2& value) { return {static_cast<float>(value.x_), static_cast<float>(value.y_)}; };
inline ImVec4 ToImGui(const Vector4& value) { return {value.x_, value.y_, value.z_, value.w_}; }
inline ImVec4 ToImGui(const Color& value) { return {value.r_, value.g_, value.b_, value.a_}; }
inline ImRect ToImGui(const IntRect& rect) { return { ToImGui(rect.Min()), ToImGui(rect.Max()) }; }

inline Vector2 ToVector2(const ImVec2& value) { return {value.x, value.y}; }
inline Vector4 ToVector4(const ImVec4& value) { return {value.x, value.y, value.z, value.w}; }
inline Color ToColor(const ImVec4& value) { return {value.x, value.y, value.z, value.w}; }
inline IntVector2 ToIntVector2(const ImVec2& value) { return {RoundToInt(value.x), RoundToInt(value.y)}; }
inline IntRect ToIntRect(const ImRect& value) { return {ToIntVector2(value.Min), ToIntVector2(value.Max)}; }


template<>
inline bool Render<String>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    auto attrAccesor = attr->AccesorCast<String>();

    String value;
    attrAccesor->Get(&value);
    ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText(label, &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        attrAccesor->Set(value);
        return true;
    }

    return false;
}

template<>
inline bool Render<StringVector>(const char* label, AttributePtr& attr, const EditOptions& options)
{
    //options.
    auto attrAccesor = attr->AccesorCast<StringVector>();

    bool modified = false;

    StringVector value;
    attrAccesor->Get(&value);

    auto idHeader = format("{}###{:X}.header"
        , format("List [{}]",  value.size())
        , label
        );

    bool isOpen = ImGui::CollapsingHeader(idHeader.c_str());

    if (!isOpen && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", Se::String::joined(value, "\n").c_str());

    if (!isOpen)
        return false;

    //TODO Move to stack,
    static std::unordered_map<Se::String, Se::String> bufferEditor = {};

    int i = 0;
    for (auto& str : value)
    {
        auto itemId = format("###{}.{}", label, i++);
        String tmp = str;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 25);
        if (ImGui::InputText(itemId.c_str(), &tmp)) {
            modified = true;
            str = tmp;
        }

        ImGui::SameLine();
        ColorScopeGuard guard({
            {ImGuiCol_Button, ToImGui(Color(.0f))}
        });
        if (ImGui::Button(format("{}{}.delete", FontAwesomeIcons::FA_TIMES, itemId).c_str()))
        {
            modified = true;
            value.erase(value.begin() + i); // remo(std::move(buff));
        }
    }

    ImGui::Separator();
    auto itemId = format("###{}.buffer", label, i++);
    auto& buff = bufferEditor[label];
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 25);

    const bool isTextClicked = ImGui::InputText(itemId.c_str(), &buff, ImGuiInputTextFlags_EnterReturnsTrue);
    bool isButtonClicked{false};

    ImGui::SameLine();
    {
        ColorScopeGuard guard({
            {ImGuiCol_Text, ToImGui(Color::GREEN)},
            {ImGuiCol_Button, ToImGui(Color(.0f))}
        });
        isButtonClicked = ImGui::Button(format("{}###{}.add", FontAwesomeIcons::FA_PLUS, label).c_str());
    }

    if (isTextClicked || isButtonClicked) {
        modified = true;
        value.push_back(std::move(buff));
    }

    if (modified)
        attrAccesor->Set(value);

    return modified;
}




} // namespace Se
