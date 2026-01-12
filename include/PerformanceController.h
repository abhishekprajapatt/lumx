#pragma once

#include "..\include\Common.h"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace LumX
{
    class PerformanceController
    {
    public:
        PerformanceController();
        ~PerformanceController();

        HRESULT Initialize();
        void Shutdown();

        void Update(float deltaTime);
        void OnSystemStateChange();
        void SetLowPowerMode(bool enabled);
        void SetFullscreenMode(bool enabled);

        bool ShouldReduceAnimations() const;
        bool IsFullscreenAppActive() const;
        bool IsLowPowerMode() const;

        float GetAnimationSpeedMultiplier() const;

    private:
        enum class SystemState
        {
            Normal,
            BatterySaver,
            Fullscreen
        };

        SystemState m_systemState;
        bool m_lowPowerMode;
        bool m_fullscreenActive;
        std::atomic<float> m_cpuUsage;
        std::atomic<float> m_memoryUsage;
        std::thread m_monitorThread;
        std::atomic<bool> m_running;
        std::mutex m_stateMutex;

        void MonitorSystemResources();
        SystemState DetermineSystemState();
        HRESULT CheckBatteryStatus();
        HRESULT CheckActiveWindow();
    };
}
