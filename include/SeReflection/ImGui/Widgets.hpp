#pragma once

#include "SeMath/MathDefs.hpp"
#include "imgui_internal.h"
#include <Se/String.hpp>
#include <imgui.h>
#include <span>

namespace Se {

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

namespace Widget {

inline String GetFormatStringForStep(double step)
{
    if (step >= 1.0 || step <= 0.0)
        return "%.0f";
    else {
        const auto numDigits = std::clamp(static_cast<int>(std::round(-std::log10(step))), 1, 8);
        return format("%.{}f", numDigits);
    }
};

inline unsigned GetFloatNumberOfDigits(std::span<const float> values, const EditOptions& options)
{
    if (options.step_ >= 1.0 || options.step_ <= 0.0)
        return 0;

    int result = RoundToInt(-std::log10(options.step_));
    for (float value : values)
    {
        const float absValue = Abs(value);
        const int numDigits = absValue != 0.0f ? RoundToInt(-std::log10(absValue)) + 1 : 0;
        result = Max(result, numDigits);
    }
    return Clamp(result, 1, 8);
}

inline String GetFloatFormatString(std::span<const float> values, const EditOptions& options)
{
    const unsigned numDigits = GetFloatNumberOfDigits(values, options);
    return format("%.{}f", numDigits);
}


inline bool EditEnum(const char* label, int& var, const Se::StringVector& items)
{
    //const auto& items = *options.intToString_;
    const auto maxEnumValue = static_cast<int>(items.size() - 1);
    bool valueChanged = false;

    int value = std::clamp(var, 0, maxEnumValue);
    //ImGui::SetNextItemWidth(options.componentWidth_ ? *options.componentWidth_ :  ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo(label, items[value].c_str()))
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

inline bool OptionalVector2(const char* label, float* value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const String format = GetFloatFormatString({value, 2}, options);
    return ImGui::DragFloat2(label, value, options.step_, options.min_, options.max_, format.c_str());
}

inline bool OptionalIntVector2(const char* label, int* value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    return ImGui::DragInt2(label, value, options.step_, static_cast<int>(options.min_), static_cast<int>(options.max_));
}

inline bool OptionalVector3(const char* label, float* value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    const String format = GetFloatFormatString({value, 3}, options);
    return ImGui::DragFloat3(label, value, options.step_, options.min_, options.max_, format.c_str());
}

inline bool OptionalIntVector3(const char* label, int* value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    return ImGui::DragInt3(label, value, options.step_, static_cast<int>(options.min_), static_cast<int>(options.max_));
}

inline bool OptionalVector4(const char* label, float* value, const EditOptions& options)
{
    //TODO add support Localization
    String title = label;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (options.asColor_) {
       return ImGui::ColorEdit4(title.c_str(), value);
    }
    const String format = GetFloatFormatString({value, 4}, options);
    return ImGui::DragFloat4(label, value, options.step_, options.min_, options.max_, format.c_str());
}

inline bool OptionalIntVector4(const char* label, int* value, const EditOptions& options)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    return ImGui::DragInt4(label, value, options.step_, static_cast<int>(options.min_), static_cast<int>(options.max_));
}



} // namespace Widget

template<class T>
bool RenderParameter(const char* label, T& value, const EditOptions& opt = {});

template<>
inline bool RenderParameter(const char* label, ImVec2& value, const EditOptions& opt) {
    return Widget::OptionalVector2(label, &value.x, opt); }


template<>
inline bool RenderParameter(const char* label, ImVec4& value, const EditOptions& options) {
    return Widget::OptionalVector4(label, &value.x, options); }

// template<>
// inline bool RenderParameter(const char* label, ImRect& value, const EditOptions& options) {
//     return Widget::RenderVec4(label, &value.x, options); }



} // namespace Se