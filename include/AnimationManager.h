#pragma once

#include "..\include\Common.h"

namespace LumX
{
    class AnimationManager
    {
    public:
        static AnimationManager &GetInstance()
        {
            static AnimationManager instance;
            return instance;
        }

        AnimationManager(const AnimationManager &) = delete;
        AnimationManager &operator=(const AnimationManager &) = delete;

        using AnimationCallback = std::function<void(float)>;

        HRESULT Initialize();
        void Shutdown();

        void Update(float deltaTime);

        int CreateAnimation(float duration, const AnimationCallback &callback);
        void CancelAnimation(int animationId);
        void PauseAnimation(int animationId);
        void ResumeAnimation(int animationId);

        float EaseLinear(float t);
        float EaseInQuad(float t);
        float EaseOutQuad(float t);
        float EaseInOutQuad(float t);
        float EaseInCubic(float t);
        float EaseOutCubic(float t);
        float EaseInOutCubic(float t);
        float EaseInQuart(float t);
        float EaseOutQuart(float t);
        float EaseInOutQuart(float t);

    private:
        struct Animation
        {
            int id;
            float duration;
            float elapsed;
            AnimationCallback callback;
            bool paused;
            bool active;
        };

        AnimationManager();
        ~AnimationManager();

        std::vector<Animation> m_animations;
        std::mutex m_animationMutex;
        int m_nextAnimationId;
    };
}
