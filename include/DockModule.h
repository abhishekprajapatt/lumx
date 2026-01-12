#pragma once

#include "..\include\Common.h"
#include "..\include\PerformanceController.h"

namespace LumX
{
    class DockModule : public IModule
    {
    public:
        DockModule(PerformanceController *perfController);
        ~DockModule();

        HRESULT Initialize() override;
        void Shutdown() override;
        void Update(float deltaTime) override;
        void OnMouseMove(const Point2D &pos) override;
        void OnMouseLeave() override;
        void OnDisplayChange() override;
        void OnDPIChange() override;
        void SetEnabled(bool enabled) override;
        bool IsEnabled() const override;

        void RefreshApplicationList();

    private:
        struct DockIcon
        {
            std::wstring appName;
            std::wstring appPath;
            HICON iconHandle;
            int index;
            Point2D position;
            float scale;
            float targetScale;
            bool pinned;
            HWND appHwnd;
        };

        HWND m_dockWindow;
        ComPtr<ID2D1Factory> m_d2dFactory;
        ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
        ComPtr<ID2D1SolidColorBrush> m_backgroundBrush;
        ComPtr<IDCompositionDevice> m_dcompDevice;
        ComPtr<IDCompositionVisual> m_dockVisual;

        std::vector<std::unique_ptr<DockIcon>> m_icons;
        Point2D m_dockPosition;
        Size2D m_dockSize;
        Point2D m_lastMousePos;
        float m_hideTimer;
        bool m_isHidden;
        bool m_isVisible;
        bool m_enabled;
        PerformanceController *m_perfController;
        float m_dpi;
        std::mutex m_iconMutex;

        static LRESULT CALLBACK DockWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        HRESULT CreateRenderTarget();
        HRESULT LoadApplications();
        HRESULT ExtractAppIcon(const std::wstring &appPath, HICON &icon);
        void UpdateDockPosition();
        void UpdateIconPositions();
        void UpdateIconAnimations(float deltaTime);
        void ShowDock();
        void HideDock();
        void RenderDock();
        void RefreshIconLayout();

        Point2D CalculateIconPosition(int index, float scale);
        float CalculateZoomScale(const Point2D &mousePos, int iconIndex);
        bool IsMouseNearDock(const Point2D &pos);
    };
}
