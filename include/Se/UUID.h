#pragma once

#include <Se/String.hpp>

#if defined(_WIN32)
#  include <windows.h>
#  include <io.h>
#endif

#if defined(__ANDROID__)
#include <jni.h>
#include <SDL.h>
#elif defined(__linux__)
#include <pwd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <uuid/uuid.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <SystemConfiguration/SystemConfiguration.h> // For the detection functions inside GetLoginName().
#endif

namespace Se {

/// Return a random UUID.
inline String GenerateUUID()
{
#if _WIN32
    UUID uuid{};
    RPC_CSTR str = nullptr;

    UuidCreate(&uuid);
    UuidToStringA(&uuid, &str);

    std::string result(reinterpret_cast<const char*>(str));
    RpcStringFreeA(&str);
    return result;
#elif ANDROID
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

    auto cls = env->FindClass("java/util/UUID");
    auto randomUUID = env->GetStaticMethodID(cls, "randomUUID", "()Ljava/util/UUID;");
    auto getMost = env->GetMethodID(cls, "getMostSignificantBits", "()J");
    auto getLeast = env->GetMethodID(cls, "getLeastSignificantBits", "()J");

    jobject uuid = env->CallStaticObjectMethod(cls, randomUUID);
    jlong upper = env->CallLongMethod(uuid, getMost);
    jlong lower = env->CallLongMethod(uuid, getLeast);

    env->DeleteLocalRef(uuid);
    env->DeleteLocalRef(cls);

    char str[37]{};
    snprintf(str, sizeof(str), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        (uint8_t)(upper >> 56), (uint8_t)(upper >> 48), (uint8_t)(upper >> 40), (uint8_t)(upper >> 32),
        (uint8_t)(upper >> 24), (uint8_t)(upper >> 16), (uint8_t)(upper >> 8), (uint8_t)upper,
        (uint8_t)(lower >> 56), (uint8_t)(lower >> 48), (uint8_t)(lower >> 40), (uint8_t)(lower >> 32),
        (uint8_t)(lower >> 24), (uint8_t)(lower >> 16), (uint8_t)(lower >> 8), (uint8_t)lower
    );

    return {str};
#elif __APPLE__
    auto guid = CFUUIDCreate(NULL);
    auto bytes = CFUUIDGetUUIDBytes(guid);
    CFRelease(guid);

    char str[37]{};
    snprintf(str, sizeof(str), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        bytes.byte0, bytes.byte1, bytes.byte2, bytes.byte3, bytes.byte4, bytes.byte5, bytes.byte6, bytes.byte7,
        bytes.byte8, bytes.byte9, bytes.byte10, bytes.byte11, bytes.byte12, bytes.byte13, bytes.byte14, bytes.byte15
    );

    return {str};
#else
    uuid_t uuid{};
    char str[37]{};

    uuid_generate(uuid);
    uuid_unparse(uuid, str);
    return {str};
#endif
}

typename std::size_t UUID;

}