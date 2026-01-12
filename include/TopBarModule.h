#pragma once

#include "..\include\Common.h"
#include "..\include\PerformanceController.h"

namespace LumX
{
    class TopBarModule : public IModule
    {
    public:
        TopBarModule(PerformanceController *perfController);
        ~TopBarModule();

        HRESULT Initialize() override;
        void Shutdown() override;
        void Update(float deltaTime) override;
        void OnMouseMove(const Point2D &pos) override;
        void OnMouseLeave() override;
        void OnDisplayChange() override;
        void OnDPIChange() override;
        void SetEnabled(bool enabled) override;
        bool IsEnabled() const override;

    private:
        struct SystemStatus
        {
            std::wstring activeAppName;
            std::wstring timeString;
            int batteryPercent;
            bool batteryCharging;
            bool wifiConnected;
            bool soundMuted;
            int volumePercent;
            float cpuUsage;
            float memoryUsage;
        };

        HWND m_topBarWindow;
        ComPtr<ID2D1Factory> m_d2dFactory;
        ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
        ComPtr<IDWriteFactory> m_dwFactory;
        ComPtr<IDWriteTextFormat> m_textFormat;
        ComPtr<ID2D1SolidColorBrush> m_textBrush;
        ComPtr<ID2D1SolidColorBrush> m_backgroundBrush;
        ComPtr<IDCompositionDevice> m_dcompDevice;
        ComPtr<IDCompositionVisual> m_topBarVisual;

        SystemStatus m_systemStatus;
        Point2D m_topBarPosition;
        Size2D m_topBarSize;
        bool m_enabled;
        PerformanceController *m_perfController;
        float m_dpi;
        std::mutex m_statusMutex;
        float m_updateTimer;

        static LRESULT CALLBACK TopBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        HRESULT CreateRenderTarget();
        void UpdateSystemStatus();
        void RenderTopBar();
        void UpdateTopBarPosition();
        std::wstring GetActiveWindowName();
        HRESULT QueryBatteryStatus();
        HRESULT QueryNetworkStatus();
        HRESULT QueryAudioStatus();
        std::wstring FormatTime();
    };
}
