#include "..\include\AnimationManager.h"

namespace LumX
{
    AnimationManager::AnimationManager() : m_nextAnimationId(0)
    {
    }

    AnimationManager::~AnimationManager()
    {
    }

    HRESULT AnimationManager::Initialize()
    {
        return S_OK;
    }

    void AnimationManager::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);
        m_animations.clear();
    }

    void AnimationManager::Update(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);

        for (auto &anim : m_animations)
        {
            if (!anim.active || anim.paused)
                continue;

            anim.elapsed += deltaTime;
            float progress = anim.elapsed / anim.duration;

            if (progress >= 1.0f)
            {
                anim.callback(1.0f);
                anim.active = false;
            }
            else
            {
                anim.callback(progress);
            }
        }

        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(),
                           [](const Animation &a)
                           { return !a.active; }),
            m_animations.end());
    }

    int AnimationManager::CreateAnimation(float duration, const AnimationCallback &callback)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);

        Animation anim;
        anim.id = m_nextAnimationId++;
        anim.duration = duration;
        anim.elapsed = 0.0f;
        anim.callback = callback;
        anim.paused = false;
        anim.active = true;

        m_animations.push_back(anim);
        return anim.id;
    }

    void AnimationManager::CancelAnimation(int animationId)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);
        for (auto &anim : m_animations)
        {
            if (anim.id == animationId)
            {
                anim.active = false;
                break;
            }
        }
    }

    void AnimationManager::PauseAnimation(int animationId)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);
        for (auto &anim : m_animations)
        {
            if (anim.id == animationId)
            {
                anim.paused = true;
                break;
            }
        }
    }

    void AnimationManager::ResumeAnimation(int animationId)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);
        for (auto &anim : m_animations)
        {
            if (anim.id == animationId)
            {
                anim.paused = false;
                break;
            }
        }
    }

    float AnimationManager::EaseLinear(float t)
    {
        return t;
    }

    float AnimationManager::EaseInQuad(float t)
    {
        return t * t;
    }

    float AnimationManager::EaseOutQuad(float t)
    {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    float AnimationManager::EaseInOutQuad(float t)
    {
        if (t < 0.5f)
        {
            return 2.0f * t * t;
        }
        return -1.0f + (4.0f - 2.0f * t) * t;
    }

    float AnimationManager::EaseInCubic(float t)
    {
        return t * t * t;
    }

    float AnimationManager::EaseOutCubic(float t)
    {
        return 1.0f + (t - 1.0f) * (t - 1.0f) * (t - 1.0f);
    }

    float AnimationManager::EaseInOutCubic(float t)
    {
        if (t < 0.5f)
        {
            return 4.0f * t * t * t;
        }
        return 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    float AnimationManager::EaseInQuart(float t)
    {
        return t * t * t * t;
    }

    float AnimationManager::EaseOutQuart(float t)
    {
        return 1.0f - std::pow(1.0f - t, 4.0f);
    }

    float AnimationManager::EaseInOutQuart(float t)
    {
        if (t < 0.5f)
        {
            return 8.0f * t * t * t * t;
        }
        return 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;
    }
}
