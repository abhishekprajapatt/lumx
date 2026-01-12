#pragma once

#include <windows.h>
#include <winuser.h>
#include <shellapi.h>
#include <shlobj.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <dcomp.h>
#include <d3d11.h>
#include <dwrite.h>
#include <wrl.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace LumX
{
    const int DOCK_HEIGHT = 80;
    const int DOCK_PADDING = 16;
    const int DOCK_ICON_SIZE = 48;
    const int TOPBAR_HEIGHT = 28;
    const float ZOOM_SCALE = 1.5f;
    const int ANIMATION_DURATION_MS = 300;
    const int HIDE_TIMEOUT_MS = 2000;
    const int CHECK_INTERVAL_MS = 50;

    struct Point2D
    {
        float x;
        float y;
        Point2D(float x = 0, float y = 0) : x(x), y(y) {}
        Point2D operator+(const Point2D &other) const
        {
            return Point2D(x + other.x, y + other.y);
        }
        Point2D operator-(const Point2D &other) const
        {
            return Point2D(x - other.x, y - other.y);
        }
        Point2D operator*(float scalar) const
        {
            return Point2D(x * scalar, y * scalar);
        }
        float distance(const Point2D &other) const
        {
            float dx = x - other.x;
            float dy = y - other.y;
            return std::sqrt(dx * dx + dy * dy);
        }
    };

    struct Size2D
    {
        float width;
        float height;
        Size2D(float w = 0, float h = 0) : width(w), height(h) {}
    };

    struct Rect2D
    {
        float left;
        float top;
        float right;
        float bottom;
        Rect2D(float l = 0, float t = 0, float r = 0, float b = 0)
            : left(l), top(t), right(r), bottom(b) {}
        float width() const { return right - left; }
        float height() const { return bottom - top; }
        bool contains(const Point2D &p) const
        {
            return p.x >= left && p.x <= right && p.y >= top && p.y <= bottom;
        }
    };

    class IModule
    {
    public:
        virtual ~IModule() = default;
        virtual HRESULT Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void OnMouseMove(const Point2D &pos) = 0;
        virtual void OnMouseLeave() = 0;
        virtual void OnDisplayChange() = 0;
        virtual void OnDPIChange() = 0;
        virtual void SetEnabled(bool enabled) = 0;
        virtual bool IsEnabled() const = 0;
    };
}
