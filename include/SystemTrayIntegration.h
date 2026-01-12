#pragma once

#include "..\include\Common.h"

namespace LumX
{
    class SystemTrayIntegration
    {
    public:
        SystemTrayIntegration();
        ~SystemTrayIntegration();

        HRESULT Initialize(HWND hwnd);
        void Shutdown();

        HRESULT AddTrayIcon(HICON icon, const std::wstring &tooltip);
        HRESULT RemoveTrayIcon();
        HRESULT UpdateTrayIcon(HICON icon, const std::wstring &tooltip);

    private:
        HWND m_hwnd;
        bool m_initialized;
        NOTIFYICONDATAW m_nid;

        static const UINT WM_TRAYICON = WM_APP + 1;
    };
}
