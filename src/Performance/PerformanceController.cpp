#include "..\include\PerformanceController.h"
#include <powrprof.h>
#include <winbase.h>

#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

namespace LumX
{
    PerformanceController::PerformanceController()
        : m_systemState(SystemState::Normal),
          m_lowPowerMode(false),
          m_fullscreenActive(false),
          m_cpuUsage(0.0f),
          m_memoryUsage(0.0f),
          m_running(false)
    {
    }

    PerformanceController::~PerformanceController()
    {
        Shutdown();
    }

    HRESULT PerformanceController::Initialize()
    {
        m_running = true;
        m_monitorThread = std::thread([this]()
                                      { MonitorSystemResources(); });
        return S_OK;
    }

    void PerformanceController::Shutdown()
    {
        m_running = false;
        if (m_monitorThread.joinable())
        {
            m_monitorThread.join();
        }
    }

    void PerformanceController::Update(float deltaTime)
    {
        SystemState newState = DetermineSystemState();
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_systemState = newState;
        }
    }

    void PerformanceController::OnSystemStateChange()
    {
        Update(0.0f);
    }

    void PerformanceController::SetLowPowerMode(bool enabled)
    {
        m_lowPowerMode = enabled;
    }

    void PerformanceController::SetFullscreenMode(bool enabled)
    {
        m_fullscreenActive = enabled;
    }

    bool PerformanceController::ShouldReduceAnimations() const
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        return m_lowPowerMode || m_fullscreenActive || m_systemState == SystemState::BatterySaver;
    }

    bool PerformanceController::IsFullscreenAppActive() const
    {
        return m_fullscreenActive;
    }

    bool PerformanceController::IsLowPowerMode() const
    {
        return m_lowPowerMode;
    }

    float PerformanceController::GetAnimationSpeedMultiplier() const
    {
        if (ShouldReduceAnimations())
        {
            return 0.3f;
        }
        return 1.0f;
    }

    void PerformanceController::MonitorSystemResources()
    {
        while (m_running)
        {
            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
            {
                ULARGE_INTEGER kernel, user;
                kernel.LowPart = kernelTime.dwLowDateTime;
                kernel.HighPart = kernelTime.dwHighDateTime;
                user.LowPart = userTime.dwLowDateTime;
                user.HighPart = userTime.dwHighDateTime;

                ULONGLONG totalTime = kernel.QuadPart + user.QuadPart;
                m_cpuUsage = static_cast<float>(totalTime) / 10000000.0f;
            }

            MEMORYSTATUSEX memStatus;
            memStatus.dwLength = sizeof(MEMORYSTATUSEX);
            if (GlobalMemoryStatusEx(&memStatus))
            {
                m_memoryUsage = (100.0f * (memStatus.ullTotalPhys - memStatus.ullAvailPhys)) / memStatus.ullTotalPhys;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    PerformanceController::SystemState PerformanceController::DetermineSystemState()
    {
        if (m_fullscreenActive)
        {
            return SystemState::Fullscreen;
        }

        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus))
        {
            if (powerStatus.SystemStatusFlag == 1)
            {
                return SystemState::BatterySaver;
            }
        }

        return SystemState::Normal;
    }

    HRESULT PerformanceController::CheckBatteryStatus()
    {
        SYSTEM_POWER_STATUS powerStatus;
        if (!GetSystemPowerStatus(&powerStatus))
        {
            return E_FAIL;
        }
        return S_OK;
    }

    HRESULT PerformanceController::CheckActiveWindow()
    {
        HWND activeWindow = GetForegroundWindow();
        if (!activeWindow)
        {
            return E_FAIL;
        }

        RECT windowRect;
        GetWindowRect(activeWindow, &windowRect);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if ((windowRect.right - windowRect.left == screenWidth) &&
            (windowRect.bottom - windowRect.top == screenHeight))
        {
            m_fullscreenActive = true;
        }
        else
        {
            m_fullscreenActive = false;
        }

        return S_OK;
    }
}
