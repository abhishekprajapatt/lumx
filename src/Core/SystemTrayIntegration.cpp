#include "..\include\SystemTrayIntegration.h"
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

namespace LumX
{
    SystemTrayIntegration::SystemTrayIntegration()
        : m_hwnd(nullptr),
          m_initialized(false)
    {
        ZeroMemory(&m_nid, sizeof(m_nid));
    }

    SystemTrayIntegration::~SystemTrayIntegration()
    {
        Shutdown();
    }

    HRESULT SystemTrayIntegration::Initialize(HWND hwnd)
    {
        m_hwnd = hwnd;

        m_nid.cbSize = sizeof(NOTIFYICONDATAW);
        m_nid.hWnd = hwnd;
        m_nid.uID = 1;
        m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        m_nid.uCallbackMessage = WM_TRAYICON;
        m_nid.uVersion = NOTIFYICON_VERSION_4;

        if (!Shell_NotifyIconW(NIM_ADD, &m_nid))
        {
            return E_FAIL;
        }

        m_initialized = true;
        return S_OK;
    }

    void SystemTrayIntegration::Shutdown()
    {
        if (m_initialized && m_hwnd)
        {
            Shell_NotifyIconW(NIM_DELETE, &m_nid);
            m_initialized = false;
        }
    }

    HRESULT SystemTrayIntegration::AddTrayIcon(HICON icon, const std::wstring &tooltip)
    {
        if (!m_initialized)
        {
            return E_NOT_INITIALIZED;
        }

        m_nid.hIcon = icon;
        wcscpy_s(m_nid.szTip, tooltip.c_str());

        if (!Shell_NotifyIconW(NIM_ADD, &m_nid))
        {
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT SystemTrayIntegration::RemoveTrayIcon()
    {
        if (!m_initialized)
        {
            return E_NOT_INITIALIZED;
        }

        if (!Shell_NotifyIconW(NIM_DELETE, &m_nid))
        {
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT SystemTrayIntegration::UpdateTrayIcon(HICON icon, const std::wstring &tooltip)
    {
        if (!m_initialized)
        {
            return E_NOT_INITIALIZED;
        }

        m_nid.hIcon = icon;
        wcscpy_s(m_nid.szTip, tooltip.c_str());

        if (!Shell_NotifyIconW(NIM_MODIFY, &m_nid))
        {
            return E_FAIL;
        }

        return S_OK;
    }
}
