#include "Util/Animation.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

namespace Util {
Animation::Animation(const std::vector<std::string> &paths, bool play,
                     std::size_t interval, bool looping, std::size_t cooldown)
    : m_State(play ? State::PLAY : State::PAUSE),
      m_Interval(interval),
      m_Looping(looping),
      m_Cooldown(cooldown) {
    m_Frames.reserve(paths.size());
    for (const auto &path : paths) {
        m_Frames.push_back(std::make_shared<Util::Image>(path));
    }
}

void Animation::SetCurrentFrame(std::size_t index) {
    m_Index = index;
    if (m_State == State::ENDED || m_State == State::COOLDOWN) {
        /*this make sure if user setframe on ENDED/COOLDOWN, will play from
         * where you set the frame*/
        m_IsChangeFrame = true;
    }
}

void Animation::Draw(const Core::Matrices &data) {
    m_Frames[m_Index]->Draw(data);
    Update();
}

void Animation::Play() {
    if (m_State == State::PLAY)
        return;
    if (m_State == State::ENDED || m_State == State::COOLDOWN) {
        m_Index = m_IsChangeFrame ? m_Index : 0;
        m_IsChangeFrame = false;
    }
    m_State = State::PLAY;
}

void Animation::Pause() {
    if (m_State == State::PLAY || m_State == State::COOLDOWN) {
        m_State = State::PAUSE;
    }
}

void Animation::Update() {
    unsigned long nowTime = Util::Time::GetElapsedTimeMs();
    if (m_State == State::PAUSE || m_State == State::ENDED) {
        LOG_TRACE("[ANI] is pause");
        return;
    }

    if (m_State == State::COOLDOWN) {
        if (nowTime >= m_CooldownEndTime) {
            Play();
        }
        return;
    }

    m_TimeBetweenFrameUpdate += Util::Time::GetDeltaTimeMs();
    auto updateFrameCount =
        static_cast<unsigned int>(m_TimeBetweenFrameUpdate / m_Interval);
    if (updateFrameCount <= 0)
        return;

    // FIX 1: Keep the remainder so timing drift doesn't accumulate.
    // Previously this was set to 0, which discarded sub-interval time and
    // caused frames to be skipped on slower machines.
    m_TimeBetweenFrameUpdate -= updateFrameCount * m_Interval;

    m_Index += updateFrameCount;

    unsigned int const totalFramesCount = m_Frames.size();
    if (m_Index >= totalFramesCount) {
        if (m_Looping) {
            // FIX 2: When cooldown is 0, loop instantly back to frame 0 and
            // stay in PLAY state. The original code always went through
            // COOLDOWN and sat on the last frame for one Draw() call, causing
            // a one-frame flicker at the end of every walk cycle.
            if (m_Cooldown == 0) {
                m_Index = 0;
                // m_State stays PLAY — no flicker, no state transition needed.
            } else {
                m_Index = m_Frames.size() - 1;
                m_CooldownEndTime = nowTime + m_Cooldown;
                m_State = State::COOLDOWN;
            }
        } else {
            m_Index = m_Frames.size() - 1;
            m_State = State::ENDED;
        }
    }
}
} // namespace Util