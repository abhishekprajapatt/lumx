#pragma once

#include "..\include\Common.h"

namespace LumX
{
    class AutoStartManager
    {
    public:
        static HRESULT EnableAutoStart(const std::wstring &appPath);
        static HRESULT DisableAutoStart();
        static bool IsAutoStartEnabled();

    private:
        static const wchar_t *REGISTRY_RUN_KEY;
        static const wchar_t *APP_NAME;
    };
}
