#pragma once

#define LUMX_VERSION_MAJOR 1
#define LUMX_VERSION_MINOR 0
#define LUMX_VERSION_PATCH 0
#define LUMX_VERSION_BUILD 1

#define LUMX_VERSION_STRING "1.0.0.1"
#define LUMX_PRODUCT_NAME "LumX - Windows macOS-like UI Framework"
#define LUMX_COMPANY_NAME "LumX Project"
#define LUMX_FILE_DESCRIPTION "Production-ready Windows UI framework for macOS-like experience"
#define LUMX_INTERNAL_NAME "LumX"
#define LUMX_ORIGINAL_NAME "LumX.exe"
#define LUMX_COPYRIGHT "Copyright (c) 2024 LumX Project. All rights reserved."

namespace LumX
{
    class VersionInfo
    {
    public:
        static constexpr int MajorVersion = LUMX_VERSION_MAJOR;
        static constexpr int MinorVersion = LUMX_VERSION_MINOR;
        static constexpr int PatchVersion = LUMX_VERSION_PATCH;
        static constexpr int BuildVersion = LUMX_VERSION_BUILD;

        static const char *GetVersionString()
        {
            return LUMX_VERSION_STRING;
        }

        static const char *GetProductName()
        {
            return LUMX_PRODUCT_NAME;
        }

        static bool IsDebugBuild()
        {
#ifdef _DEBUG
            return true;
#else
            return false;
#endif
        }

        static const char *GetBuildConfiguration()
        {
#ifdef _DEBUG
            return "Debug";
#else
            return "Release";
#endif
        }
    };
}
