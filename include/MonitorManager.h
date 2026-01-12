#pragma once

#include "..\include\Common.h"

namespace LumX
{
    struct MonitorInfo
    {
        HMONITOR hMonitor;
        std::wstring name;
        Rect2D bounds;
        Rect2D workArea;
        float dpi;
        bool isPrimary;
        UINT id;
    };

    class MonitorManager
    {
    public:
        static MonitorManager &GetInstance()
        {
            static MonitorManager instance;
            return instance;
        }

        MonitorManager(const MonitorManager &) = delete;
        MonitorManager &operator=(const MonitorManager &) = delete;

        HRESULT Initialize();
        void Shutdown();

        const std::vector<MonitorInfo> &GetMonitors() const;
        MonitorInfo GetPrimaryMonitor() const;
        MonitorInfo GetMonitorFromPoint(const Point2D &point);
        MonitorInfo GetMonitorFromHwnd(HWND hwnd);

        float GetDPIForMonitor(HMONITOR hMonitor);
        void RefreshMonitors();

    private:
        MonitorManager();
        ~MonitorManager();

        std::vector<MonitorInfo> m_monitors;
        std::mutex m_monitorMutex;

        static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lpRect, LPARAM lParam);
    };
}
