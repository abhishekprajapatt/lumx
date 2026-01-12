#include "..\include\MonitorManager.h"
#include <shcore.h>

#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "user32.lib")

namespace LumX
{
    MonitorManager::MonitorManager()
    {
    }

    MonitorManager::~MonitorManager()
    {
    }

    HRESULT MonitorManager::Initialize()
    {
        RefreshMonitors();
        return S_OK;
    }

    void MonitorManager::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        m_monitors.clear();
    }

    const std::vector<MonitorInfo> &MonitorManager::GetMonitors() const
    {
        return m_monitors;
    }

    MonitorInfo MonitorManager::GetPrimaryMonitor() const
    {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        for (const auto &monitor : m_monitors)
        {
            if (monitor.isPrimary)
            {
                return monitor;
            }
        }

        if (!m_monitors.empty())
        {
            return m_monitors[0];
        }

        return MonitorInfo();
    }

    MonitorInfo MonitorManager::GetMonitorFromPoint(const Point2D &point)
    {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        for (const auto &monitor : m_monitors)
        {
            if (monitor.bounds.contains(point))
            {
                return monitor;
            }
        }

        return GetPrimaryMonitor();
    }

    MonitorInfo MonitorManager::GetMonitorFromHwnd(HWND hwnd)
    {
        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        for (const auto &monitor : m_monitors)
        {
            if (monitor.hMonitor == hMonitor)
            {
                return monitor;
            }
        }

        return GetPrimaryMonitor();
    }

    float MonitorManager::GetDPIForMonitor(HMONITOR hMonitor)
    {
        UINT dpiX = 96, dpiY = 96;
        if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
        {
            return static_cast<float>(dpiX);
        }
        return 96.0f;
    }

    void MonitorManager::RefreshMonitors()
    {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        m_monitors.clear();

        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
    }

    BOOL CALLBACK MonitorManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lpRect, LPARAM lParam)
    {
        MonitorManager *pThis = reinterpret_cast<MonitorManager *>(lParam);
        if (!pThis)
        {
            return TRUE;
        }

        MonitorInfo info;
        info.hMonitor = hMonitor;

        MONITORINFOEXW monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFOEXW);
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
        {
            info.bounds = Rect2D(
                static_cast<float>(monitorInfo.rcMonitor.left),
                static_cast<float>(monitorInfo.rcMonitor.top),
                static_cast<float>(monitorInfo.rcMonitor.right),
                static_cast<float>(monitorInfo.rcMonitor.bottom));

            info.workArea = Rect2D(
                static_cast<float>(monitorInfo.rcWork.left),
                static_cast<float>(monitorInfo.rcWork.top),
                static_cast<float>(monitorInfo.rcWork.right),
                static_cast<float>(monitorInfo.rcWork.bottom));

            info.isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
            info.name = monitorInfo.szDevice;
            info.dpi = pThis->GetDPIForMonitor(hMonitor);
        }

        pThis->m_monitors.push_back(info);
        return TRUE;
    }
}
