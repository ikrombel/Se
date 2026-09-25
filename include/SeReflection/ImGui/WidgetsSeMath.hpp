#pragma once

#include <SeReflection/ImGui/Widgets.hpp>

#include <SeMath/Rect.hpp>
#include <SeMath/Quaternion.hpp>

#include <Se/Timer.h>

namespace Se {


template<>
inline bool RenderParameter(const char* label, Se::Vector2& value, const EditOptions& options) {
    return Widget::OptionalVector2(label, value.data, options); }

template<>
inline bool RenderParameter(const char* label, Se::IntVector2& value, const EditOptions& options) {
    return Widget::OptionalIntVector2(label, &value.x_, options); }

template<>
inline bool RenderParameter(const char* label, Se::Vector3& value, const EditOptions& options) {
    return Widget::OptionalVector3(label, value.data, options); }

template<>
inline bool RenderParameter(const char* label, Se::IntVector3& value, const EditOptions& options) {
    return Widget::OptionalIntVector3(label, &value.x_, options); }


template<>
inline bool RenderParameter(const char* label, Se::Vector4& value, const EditOptions& options) {
    return Widget::OptionalVector4(label, value.data, options); }

template<>
inline bool RenderParameter(const char* label, Se::IntVector4& value, const EditOptions& options) {
    return Widget::OptionalIntVector4(label, &value.x_, options); }


template<>
inline bool RenderParameter(const char* label, Se::Rect& value, const EditOptions& options) {
    return Widget::OptionalVector4(label, &value.min_.x_, options); }

template<>
inline bool RenderParameter(const char* label, Se::IntRect& value, const EditOptions& options) {
    return Widget::OptionalIntVector4(label, value.data, options); }


struct QuaternionCachedInfo
{
    unsigned time_{};
    Quaternion value_;
    Vector3 angles_{Quaternion::IDENTITY.EulerAngles()};

    void PruneQuaternionCache()
    {
        static const unsigned expireTimeMs = 1000;
        const unsigned currentTime = Time::GetSystemTime();
        std::erase_if(quaternionCache, [&](const auto& pair) { 
            return currentTime - pair.second.time_ > expireTimeMs; });
    }

    Vector3 GetQuaternionAngles(ImGuiID id, const Quaternion& quaternion)
    {
        QuaternionCachedInfo& info = quaternionCache[id];

        info.time_ = Time::GetSystemTime();
        if (info.value_ == quaternion)
            return info.angles_;

        info.value_ = quaternion;
        info.angles_ = quaternion.EulerAngles();
        return info.angles_;
    }

    void UpdateQuaternionAngles(ImGuiID id, const Quaternion& quaternion, const Vector3& angles)
    {
        QuaternionCachedInfo& info = quaternionCache[id];
        info.value_ = quaternion;
        info.angles_ = angles;
    }

    std::unordered_map<ImGuiID, QuaternionCachedInfo> quaternionCache;
};

template<>
inline bool RenderParameter(const char* label, Se::Quaternion& value, const EditOptions& options)
{
    static QuaternionCachedInfo cache = {};

    const ImGuiID id = ImGui::GetID("Quaternion");
    cache.PruneQuaternionCache();

    Vector3 angles = cache.GetQuaternionAngles(id, value);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const float maxValue = 360.0f;
    if (ImGui::DragFloat3(label, &angles.x_, 1.0f, -maxValue * 100, maxValue * 100, "%.2f"))
    {
        const Quaternion newValue{angles};
        cache.UpdateQuaternionAngles(id, newValue, angles);
        return true;
    }
    return false;
}


}