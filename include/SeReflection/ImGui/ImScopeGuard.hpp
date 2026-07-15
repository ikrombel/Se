
#pragma once

#include <initializer_list>
#include <functional>
#include <imgui.h>

struct ImGuiDisabledGuard
{
    ImGuiDisabledGuard(bool disabled) {
        ImGui::BeginDisabled(disabled);
    }
    ~ImGuiDisabledGuard() {
        ImGui::EndDisabled();
    }
};

class IdScopeGuard
{
public:
    template <class T>
    explicit IdScopeGuard(const T& id)
    {
        ImGui::PushID(id);
    }

    ~IdScopeGuard()
    {
        ImGui::PopID();
    }
};

class ColorScopeGuard
{
public:
    ColorScopeGuard(ImGuiCol id, const ImVec4& color, bool enabled = true)
        : numColors_(enabled ? 1 : 0)
    {
        if (enabled)
            ImGui::PushStyleColor(id, color);
    }

    explicit ColorScopeGuard(std::initializer_list<std::pair<ImGuiCol, ImVec4>> colors, bool enabled = true)
        : numColors_(enabled ? static_cast<unsigned>(colors.size()) : 0)
    {
        if (enabled)
        {
            for (const auto& [id, color] : colors)
                ImGui::PushStyleColor(id, color);
        }
    }

    ColorScopeGuard(ImGuiCol id, ImU32 color, bool enabled = true)
        : numColors_(enabled ? 1 : 0)
    {
        if (enabled)
            ImGui::PushStyleColor(id, color);
    }

    explicit ColorScopeGuard(std::initializer_list<std::pair<ImGuiCol, ImU32>> colors, bool enabled = true)
        : numColors_(enabled ? static_cast<unsigned>(colors.size()) : 0)
    {
        if (enabled)
        {
            for (const auto& [id, color] : colors)
                ImGui::PushStyleColor(id, color);
        }
    }

    // ColorScopeGuard(ImGuiCol id, const Color& color, bool enabled = true)
    //     : numColors_(enabled ? 1 : 0)
    // {
    //     if (enabled)
    //         ImGui::PushStyleColor(id, ToImGui(color));
    // }

    // explicit ColorScopeGuard(std::initializer_list<std::pair<ImGuiCol, Color>> colors, bool enabled = true)
    //     : numColors_(enabled ? static_cast<unsigned>(colors.size()) : 0)
    // {
    //     if (enabled)
    //     {
    //         for (const auto& [id, color] : colors)
    //             ImGui::PushStyleColor(id, ToImGui(color));
    //     }
    // }

    ~ColorScopeGuard()
    {
        ImGui::PopStyleColor(numColors_);
    }

private:
    const unsigned numColors_{};
};