#pragma once

#include <string>
#include <stdint.h>
#include <fstream>

#ifdef __linux__
#include <stdlib.h>
#include <sys/utsname.h>

#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/udata.h>
#include <unicode/uversion.h>
#endif

#define SE_ARCHITECTURE_x86_32 1
#define SE_ARCHITECTURE_x86_64 2

// Find the architecture type
#if defined(__x86_64__) || defined(_M_X64)
#	define SE_ARCH_TYPE SE_ARCHITECTURE_x86_64
#else
#	define SE_ARCH_TYPE SE_ARCHITECTURE_x86_32
#endif


namespace Se {

struct GPUInfo
{
    std::string names[5];
    uint32_t numGPUs;
};

struct SystemInfo
{
    std::string cpuManufacturer;
    std::string cpuModel;
    uint32_t cpuClockSpeedMhz;
    uint32_t cpuNumCores;
    uint32_t memoryAmountMb;
    std::string osName;
    bool osIs64Bit;

    GPUInfo gpuInfo;
};

struct Platform {
    static SystemInfo getSystemInfo();

    GPUInfo sGPUInfo;
};

#if __linux__
inline SystemInfo Platform::getSystemInfo()
{
    SystemInfo output;

    // Get CPU vendor, model and number of cores
    {
        std::ifstream file("/proc/cpuinfo");
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream lineStream(line);
            std::string token;
            lineStream >> token;

            if(token == "vendor_id")
            {
                if(lineStream >> token && token == ":")
                {
                    std::string vendorId;
                    if(lineStream >> vendorId)
                        output.cpuManufacturer = vendorId.c_str();
                }
            }
            else if(token == "model")
            {
                if(lineStream >> token && token == "name")
                {
                    if (lineStream >> token && token == ":")
                    {
                        std::stringstream modelName;
                        if (lineStream >> token)
                        {
                            modelName << token;

                            while (lineStream >> token)
                                modelName << " " << token;
                        }

                        output.cpuModel = modelName.str().c_str();
                    }
                }
            }
            else if(token == "cpu")
            {
                if(lineStream >> token)
                {
                    if (token == "cores")
                    {
                        if (lineStream >> token && token == ":")
                        {
                            uint32_t numCores;
                            if (lineStream >> numCores)
                                output.cpuNumCores = numCores;
                        }
                    }
                }
            }
        }
    }

    // Get CPU frequency
    {
        std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
        uint32_t frequency;
        if(file >> frequency)
            output.cpuClockSpeedMhz = frequency / 1000;
    }

    // Get amount of system memory
    {
        std::ifstream file("/proc/meminfo");
        std::string token;
        while(file >> token)
        {
            if(token == "MemTotal:")
            {
                uint32_t memTotal;
                if(file >> memTotal)
                    output.memoryAmountMb = memTotal / 1024;
                else
                    output.memoryAmountMb = 0;

                break;
            }

            // Ignore the rest of the line
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    // Get OS version
    utsname osInfo;
    uname(&osInfo);

    // Note: This won't report the exact distro
    output.osName = std::string(osInfo.sysname) + std::string(osInfo.version);

    if (SE_ARCH_TYPE == SE_ARCHITECTURE_x86_64)
        output.osIs64Bit = true;
    else
        output.osIs64Bit = strstr(osInfo.machine, "64") != nullptr;

    // Get GPU info
    //output.gpuInfo = sGPUInfo;

    return output;
}
#endif


}