#include <screen_phase_stepping.hpp>

#include "frame_qr_layout.hpp"
#include "i18n.h"
#include "img_resources.hpp"
#include "str_utils.hpp"
#include <array>
#include <guiconfig/wizard_config.hpp>
#include <window_icon.hpp>
#include <window_qr.hpp>
#include <window_text.hpp>

namespace {
#if PRINTER_IS_PRUSA_XL
constexpr const char *QR_ADDR = "prusa.io/xl-phstep-qr";
constexpr const char *ADDR_IN_TEXT = "prusa.io/xl-phstep";
#else
    #error
#endif
constexpr const char *txt_learn_more { N_("To learn more about the phase stepping calibration process, read the article:") };
constexpr const char *txt_picking_tool { N_("Picking Tool") };
constexpr const char *txt_calibrating_x { N_("Calibrating X") };
constexpr const char *txt_calibrating_y { N_("Calibrating Y") };
constexpr const char *txt_calibration_nok { N_("Calibration of axis %c failed.\nParameter 1: forward %3d%%, backward %3d%%\nParameter 2: forward %3d%%, backward %3d%%") };
constexpr const char *txt_calibration_error { N_("Calibration failed with error.") };
constexpr const char *txt_enabling { N_("Finishing") };
constexpr const char *txt_calibration_ok { N_("Axis X vibration A reduced by %2d%%\nAxis X vibration B reduced by %2d%%\nAxis Y vibration A reduced by %2d%%\nAxis Y vibration B reduced by %2d%%") };

constexpr Rect16 radio_rect = GuiDefaults::GetButtonRect(GuiDefaults::RectScreenBody);
constexpr Rect16 inner_frame_rect = GuiDefaults::RectScreenBody - radio_rect.Height();

namespace frame {

    class CenteredText {
    protected:
        window_text_t text;

    public:
        CenteredText(window_t *parent, string_view_utf8 txt)
            : text(parent, inner_frame_rect, is_multiline::yes, is_closed_on_click_t::no, txt) {
            text.SetAlignment(Align_t::Center());
        }
    };

    class CenteredStaticText : public CenteredText {
    public:
        using CenteredText::CenteredText;
        void update(const fsm::PhaseData &) {}
    };

    class CalibrationNOK : public CenteredText {
    private:
        std::array<char, 150> text_buffer;
        char axis;

    public:
        explicit CalibrationNOK(window_t *parent, char axis)
            : CenteredText { parent, string_view_utf8::MakeNULLSTR() }
            , axis { axis } {
        }

        void update(const fsm::PhaseData &data) {
            std::array<char, 150> fmt;
            _(txt_calibration_nok).copyToRAM(fmt.data(), fmt.size());
            snprintf(text_buffer.data(), text_buffer.size(), fmt.data(), axis, data[0], data[1], data[2], data[3]);
            text.SetText(string_view_utf8::MakeRAM((const uint8_t *)text_buffer.data()));
        }
    };

    class CalibratingAxis : public CenteredStaticText {
    public:
        using CenteredStaticText::CenteredStaticText;

        // TODO add progress report
    };

    class Introduction final {
        window_text_t text;
        window_text_t link;
        window_icon_t icon_phone;
        window_qr_t qr;

    public:
        explicit Introduction(window_t *parent)
            : text { parent, FrameQRLayout::text_rect(), is_multiline::yes, is_closed_on_click_t::no, _(txt_learn_more) }
            , link { parent, FrameQRLayout::link_rect(), is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeCPUFLASH(reinterpret_cast<const uint8_t *>(ADDR_IN_TEXT)) }
            , icon_phone { parent, FrameQRLayout::phone_icon_rect(), &img::hand_qr_59x72 }
            , qr { parent, FrameQRLayout::qrcode_rect(), QR_ADDR } {
        }
        void update(const fsm::PhaseData &) {}
    };

    class PickingTool final : public CenteredStaticText {
    public:
        explicit PickingTool(window_t *parent)
            : CenteredStaticText { parent, _(txt_picking_tool) } {
        }
    };

    class CalibratingX final : public CalibratingAxis {
    public:
        explicit CalibratingX(window_t *parent)
            : CalibratingAxis { parent, _(txt_calibrating_x) } {
        }
    };

    class CalibratingY final : public CalibratingAxis {
    public:
        explicit CalibratingY(window_t *parent)
            : CalibratingAxis { parent, _(txt_calibrating_y) } {
        }
    };

    class CalibrationOK final : public CenteredText {
    private:
        std::array<char, 150> text_buffer;

    public:
        explicit CalibrationOK(window_t *parent)
            : CenteredText { parent, string_view_utf8::MakeNULLSTR() } {
        }

        void update(const fsm::PhaseData &data) {
            std::array<char, 150> fmt;
            _(txt_calibration_ok).copyToRAM(fmt.data(), fmt.size());
            snprintf(text_buffer.data(), text_buffer.size(), fmt.data(), data[0], data[1], data[2], data[3]);
            text.SetText(string_view_utf8::MakeRAM(text_buffer.data()));
        }
    };

    class Enabling final : public CenteredStaticText {
    public:
        explicit Enabling(window_t *parent)
            : CenteredStaticText { parent, _(txt_enabling) } {
        }
    };

    class CalibrationError final : public CenteredStaticText {
    public:
        explicit CalibrationError(window_t *parent)
            : CenteredStaticText { parent, _(txt_calibration_error) } {
        }
    };

    class CalibrationXNOK final : public CalibrationNOK {
    public:
        explicit CalibrationXNOK(window_t *parent)
            : CalibrationNOK { parent, 'X' } {}
    };

    class CalibrationYNOK final : public CalibrationNOK {
    public:
        explicit CalibrationYNOK(window_t *parent)
            : CalibrationNOK { parent, 'Y' } {}
    };

} // namespace frame

ScreenPhaseStepping *instance = nullptr;

PhasesPhaseStepping get_phase(const fsm::BaseData &fsm_base_data) {
    return GetEnumFromPhaseIndex<PhasesPhaseStepping>(fsm_base_data.GetPhase());
}

template <PhasesPhaseStepping Phase, class Frame>
struct FrameDefinition {
    using FrameType = Frame;
    static constexpr PhasesPhaseStepping phase = Phase;
};

template <class Storage, class... T>
struct FrameDefinitionList {
    template <class F>
    using FrameType = typename F::FrameType;

    static_assert(Storage::template has_ideal_size_for<FrameType<T>...>());

    static void create_frame(Storage &storage, PhasesPhaseStepping phase, window_t *parent) {
        auto f = [&]<typename FD> {
            if (phase == FD::phase) {
                storage.template create<typename FD::FrameType>(parent);
            }
        };
        (f.template operator()<T>(), ...);
    }

    static void destroy_frame(Storage &storage, PhasesPhaseStepping phase) {
        auto f = [&]<typename FD> {
            if (phase == FD::phase) {
                storage.template destroy<typename FD::FrameType>();
            }
        };
        (f.template operator()<T>(), ...);
    }

    static void update_frame(Storage &storage, PhasesPhaseStepping phase, const fsm::PhaseData &data) {
        auto f = [&]<typename FD> {
            if (phase == FD::phase) {
                storage.template as<typename FD::FrameType>()->update(data);
            }
        };
        (f.template operator()<T>(), ...);
    }
};

using Frames = FrameDefinitionList<ScreenPhaseStepping::FrameStorage,
    FrameDefinition<PhasesPhaseStepping::intro, frame::Introduction>,
    FrameDefinition<PhasesPhaseStepping::pick_tool, frame::PickingTool>,
    FrameDefinition<PhasesPhaseStepping::calib_x, frame::CalibratingX>,
    FrameDefinition<PhasesPhaseStepping::calib_y, frame::CalibratingY>,
    FrameDefinition<PhasesPhaseStepping::calib_x_nok, frame::CalibrationXNOK>,
    FrameDefinition<PhasesPhaseStepping::calib_y_nok, frame::CalibrationYNOK>,
    FrameDefinition<PhasesPhaseStepping::calib_error, frame::CalibrationError>,
    FrameDefinition<PhasesPhaseStepping::calib_ok, frame::CalibrationOK>,
    FrameDefinition<PhasesPhaseStepping::enabling, frame::Enabling>>;

} // namespace

ScreenPhaseStepping::ScreenPhaseStepping()
    : AddSuperWindow<screen_t> {}
    , inner_frame { this, inner_frame_rect }
    , radio(this, radio_rect, PhasesPhaseStepping::intro) {
    ClrMenuTimeoutClose();
    CaptureNormalWindow(radio);
    create_frame();
    instance = this;
}

ScreenPhaseStepping::~ScreenPhaseStepping() {
    instance = nullptr;
    ReleaseCaptureOfNormalWindow();
}

ScreenPhaseStepping *ScreenPhaseStepping::GetInstance() {
    return instance;
}

void ScreenPhaseStepping::Change(fsm::BaseData data) {
    return do_change(data);
}

void ScreenPhaseStepping::InitState(screen_init_variant var) {
    if (auto fsm_base_data = var.GetFsmBaseData()) {
        do_change(*fsm_base_data);
    }
}

screen_init_variant ScreenPhaseStepping::GetCurrentState() const {
    screen_init_variant var;
    var.SetFsmBaseData(fsm_base_data);
    return var;
}

void ScreenPhaseStepping::do_change(fsm::BaseData new_fsm_base_data) {
    if (new_fsm_base_data.GetPhase() != fsm_base_data.GetPhase()) {
        destroy_frame();
        fsm_base_data = new_fsm_base_data;
        create_frame();
        radio.Change(get_phase(fsm_base_data));
    } else {
        fsm_base_data = new_fsm_base_data;
    }
    update_frame();
}

void ScreenPhaseStepping::create_frame() {
    Frames::create_frame(frame_storage, get_phase(fsm_base_data), &inner_frame);
}

void ScreenPhaseStepping::destroy_frame() {
    Frames::destroy_frame(frame_storage, get_phase(fsm_base_data));
}

void ScreenPhaseStepping::update_frame() {
    Frames::update_frame(frame_storage, get_phase(fsm_base_data), fsm_base_data.GetData());
}
