#include "elm_equalizer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr s32 TrackTop = 45;
constexpr s32 TrackBottom = 166;
constexpr s32 TrackSpan = TrackBottom - TrackTop;

std::string formatGain(s8 gain) {
    char text[12]{};
    std::snprintf(text, sizeof(text), "%+d dB", static_cast<int>(gain));
    return text;
}

tsl::Color mutedText() {
    const tsl::Color c = tsl::defaultTextColor;
    return tsl::Color(static_cast<u8>(c.r * 3 / 5), static_cast<u8>(c.g * 3 / 5),
                      static_cast<u8>(c.b * 3 / 5), c.a);
}

} // namespace

EqualizerTuner::EqualizerTuner(const std::array<s8, BandCount>& gains,
                               std::array<std::string, BandCount> labels,
                               GainChanged gainChanged)
    : m_labels(std::move(labels)), m_gainChanged(std::move(gainChanged)) {
    setGains(gains);
    m_isItem = true;
}

void EqualizerTuner::layout(u16, u16, u16, u16) {
    setBoundaries(getX(), getY(), getWidth(), Height);
}

tsl::elm::Element* EqualizerTuner::requestFocus(tsl::elm::Element*, tsl::FocusDirection) {
    return this;
}

void EqualizerTuner::drawSeparators(tsl::gfx::Renderer* renderer) {
    renderer->drawRect(getX() + 7, getBottomBound() - 1, getWidth() - 14, 1,
                       aWithOpacity(tsl::separatorColor));
}

void EqualizerTuner::drawHighlight(tsl::gfx::Renderer* renderer) {
    renderer->drawRoundedRect(getX() + 2, getY() + 2, getWidth() - 4, Height - 4,
                              9.0f, aWithOpacity(tsl::selectionBGColor));
    if (m_editing) {
        renderer->drawRoundedRect(getX() + 5, getY() + 14, 3, Height - 28,
                                  1.5f, a(tsl::onTextColor));
    }
}

void EqualizerTuner::draw(tsl::gfx::Renderer* renderer) {
    const s32 x = getX();
    const s32 y = getY();
    const s32 width = getWidth();
    const s32 left = x + 30;
    const s32 usable = width - 60;
    const s32 column = usable / static_cast<s32>(BandCount);
    const s32 zeroY = y + TrackBottom - (12 * TrackSpan) / 24;
    const tsl::Color dim = a(mutedText());
    const tsl::Color accent = a(tsl::onTextColor);

    // Three quiet reference lines make the five controls read as one tuner,
    // while avoiding the dense graph-paper look of the previous design.
    for (const int gain : {12, 0, -12}) {
        const s32 lineY = y + TrackBottom - ((gain + 12) * TrackSpan) / 24;
        renderer->drawRect(left - 8, lineY, usable + 16, 1,
                           aWithOpacity(tsl::separatorColor));
    }

    for (std::size_t band = 0; band < BandCount; ++band) {
        const s32 cx = left + column * static_cast<s32>(band) + column / 2;
        const s8 gain = m_gains[band];
        const s32 knobY = y + TrackBottom
                        - ((static_cast<int>(gain) + 12) * TrackSpan) / 24;
        const bool selected = band == m_selected;

        if (selected) {
            renderer->drawRoundedRect(cx - 24, y + 8, 48, Height - 18, 8.0f,
                                      aWithOpacity(tsl::selectionBGColor));
        }

        const std::string gainText = formatGain(gain);
        const s32 gainWidth = static_cast<s32>(
            renderer->getTextDimensions(gainText, false, 15).first);
        renderer->drawString(gainText, false, cx - gainWidth / 2, y + 27, 15,
                             selected ? accent : dim);

        renderer->drawRoundedRect(cx - 2, y + TrackTop, 4, TrackSpan, 2.0f,
                                  aWithOpacity(tsl::separatorColor));

        const s32 fillTop = std::min(zeroY, knobY);
        const s32 fillHeight = std::max(3, std::abs(zeroY - knobY));
        renderer->drawRoundedRect(cx - 2, fillTop, 4, fillHeight, 2.0f,
                                  selected ? accent : a(tsl::trackBarFullColor));

        renderer->drawRoundedRect(cx - 13, knobY - 5, 26, 10, 5.0f,
                                  selected ? accent : a(tsl::trackBarSliderBorderColor));
        renderer->drawRoundedRect(cx - 9, knobY - 2, 18, 4, 2.0f,
                                  a(tsl::trackBarSliderColor));

        const s32 labelWidth = static_cast<s32>(
            renderer->getTextDimensions(m_labels[band], false, 15).first);
        renderer->drawString(m_labels[band], false, cx - labelWidth / 2, y + 205, 15,
                             selected ? accent : a(tsl::defaultTextColor));
    }
}

void EqualizerTuner::setGain(std::size_t band, s8 gain) {
    if (band < BandCount)
        m_gains[band] = std::clamp(gain, MinGain, MaxGain);
}

void EqualizerTuner::setGains(const std::array<s8, BandCount>& gains) {
    for (std::size_t band = 0; band < BandCount; ++band)
        setGain(band, gains[band]);
}

void EqualizerTuner::selectRelative(int delta) {
    const int count = static_cast<int>(BandCount);
    m_selected = static_cast<std::size_t>(
        (static_cast<int>(m_selected) + delta + count) % count);
    triggerNavigationFeedback();
}

void EqualizerTuner::requestGain(s8 gain) {
    gain = std::clamp(gain, MinGain, MaxGain);
    if (gain != m_gains[m_selected] && m_gainChanged)
        m_gainChanged(m_selected, gain);
}

bool EqualizerTuner::handleTuningInput(u64 keysDown, u64 keysHeld) {
    // Focus and editing are separate states. While merely focused, UP/DOWN
    // must remain available to the parent list so the user can reach the
    // settings below the tuner. A grabs the faders; A or B releases them.
    if (!m_editing) {
        if ((keysDown & KEY_A) && !(keysHeld & ~KEY_A & ALL_KEYS_MASK)) {
            m_editing = true;
            m_holdDirection = 0;
            triggerNavigationFeedback();
            return true;
        }
        return false;
    }

    if (keysDown & (KEY_A | KEY_B)) {
        m_editing = false;
        m_holdDirection = 0;
        triggerNavigationFeedback();
        return true;
    }

    // Always consume shoulder input while the tuner owns focus. Tesla also
    // uses shoulder buttons for list/page navigation, so letting the same
    // event reach the base GUI moved focus after changing the active band.
    if (keysDown & KEY_L) {
        selectRelative(-1);
        return true;
    }
    if (keysDown & KEY_R) {
        selectRelative(1);
        return true;
    }
    if (keysHeld & (KEY_L | KEY_R))
        return true;
    // The control is vertical: UP raises the selected band and DOWN lowers it.
    // L/R remain dedicated to selecting one of the five faders.
    const int direction = (keysHeld & KEY_UP) ? 1 : (keysHeld & KEY_DOWN) ? -1 : 0;
    const u64 now = ult::nowNs();
    if (direction == 0) {
        m_holdDirection = 0;
        return false;
    }

    if ((direction > 0 && (keysDown & KEY_UP))
        || (direction < 0 && (keysDown & KEY_DOWN))) {
        m_holdDirection = direction;
        m_holdStartedNs = now;
        m_lastStepNs = now;
        requestGain(static_cast<s8>(m_gains[m_selected] + direction));
        return true;
    }

    if (m_holdDirection == direction
        && now - m_holdStartedNs >= 320000000ULL
        && now - m_lastStepNs >= 75000000ULL) {
        m_lastStepNs = now;
        requestGain(static_cast<s8>(m_gains[m_selected] + direction));
        return true;
    }
    return true;
}

std::size_t EqualizerTuner::bandAt(s32 touchX) {
    const s32 left = getX() + 30;
    const s32 usable = getWidth() - 60;
    const s32 relative = std::clamp(touchX - left, 0, std::max(0, usable - 1));
    return std::min(BandCount - 1,
                    static_cast<std::size_t>(relative * static_cast<s32>(BandCount)
                                             / std::max(1, usable)));
}

s8 EqualizerTuner::gainAt(s32 touchY) {
    const s32 relative = std::clamp(touchY - (getY() + TrackTop), 0, TrackSpan);
    const float normalized = 1.0f - static_cast<float>(relative) / TrackSpan;
    return static_cast<s8>(std::lround(MinGain + normalized * (MaxGain - MinGain)));
}

bool EqualizerTuner::onTouch(tsl::elm::TouchEvent event, s32 currX, s32 currY,
                             s32, s32, s32 initialX, s32 initialY) {
    const bool initialInside = inBounds(initialX, initialY);
    if (event == tsl::elm::TouchEvent::Touch && initialInside) {
        m_dragging = true;
        m_selected = bandAt(currX);
        tsl::shiftItemFocus(this);
        requestGain(gainAt(currY));
        return true;
    }
    if (event == tsl::elm::TouchEvent::Release) {
        const bool wasDragging = m_dragging;
        m_dragging = false;
        return wasDragging;
    }
    if (m_dragging) {
        m_selected = bandAt(currX);
        requestGain(gainAt(currY));
        return true;
    }
    return false;
}
