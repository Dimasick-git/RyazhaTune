#pragma once

#include <tesla.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <string>

/**
 * One-plane five-band sound tuner.
 *
 * The widget owns only presentation state. Every gain change is handed to the
 * overlay immediately, which applies it through IPC and then calls setGain()
 * with the accepted value. This keeps a failed hardware write from leaving a
 * fake knob position on screen.
 */
class EqualizerTuner final : public tsl::elm::Element {
public:
    static constexpr std::size_t BandCount = 5;
    using GainChanged = std::function<void(std::size_t, s8)>;

    EqualizerTuner(const std::array<s8, BandCount>& gains,
                   std::array<std::string, BandCount> labels,
                   GainChanged gainChanged);

    void draw(tsl::gfx::Renderer* renderer) override;
    void drawHighlight(tsl::gfx::Renderer* renderer) override;
    void drawSeparators(tsl::gfx::Renderer* renderer) override;
    void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override;
    tsl::elm::Element* requestFocus(tsl::elm::Element* oldFocus,
                                    tsl::FocusDirection direction) override;
    bool onTouch(tsl::elm::TouchEvent event, s32 currX, s32 currY,
                 s32 prevX, s32 prevY, s32 initialX, s32 initialY) override;

    void setGain(std::size_t band, s8 gain);
    void setGains(const std::array<s8, BandCount>& gains);
    bool handleTuningInput(u64 keysDown, u64 keysHeld);

private:
    static constexpr s32 Height = 226;
    static constexpr s8 MinGain = -12;
    static constexpr s8 MaxGain = 12;

    void selectRelative(int delta);
    void requestGain(s8 gain);
    std::size_t bandAt(s32 x);
    s8 gainAt(s32 y);

    std::array<s8, BandCount> m_gains{};
    std::array<std::string, BandCount> m_labels;
    GainChanged m_gainChanged;
    std::size_t m_selected{0};
    bool m_editing{false};
    bool m_dragging{false};
    int m_holdDirection{0};
    u64 m_holdStartedNs{0};
    u64 m_lastStepNs{0};
};
