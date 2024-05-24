#include "screen_printer_setup.hpp"

#include <ScreenHandler.hpp>
#include <img_resources.hpp>

namespace screen_printer_setup_private {

constexpr Rect16 prompt_rect = Rect16::fromLTWH(0, GuiDefaults::HeaderHeight, GuiDefaults::ScreenWidth, 32);
constexpr Rect16 menu_rect = Rect16::fromLTRB(0, prompt_rect.Bottom() + 4, prompt_rect.Right(), GuiDefaults::ScreenHeight);

}; // namespace screen_printer_setup_private
using namespace screen_printer_setup_private;

MI_DONE::MI_DONE()
    : IWindowMenuItem(_("Done"), &img::ok_16x16) {
}

void MI_DONE::click(IWindowMenu &) {
    config_store().printer_setup_done.set(true);
    Screens::Access()->Close();
}

MI_NOZZLE_DIAMETER_HELP::MI_NOZZLE_DIAMETER_HELP()
    : IWindowMenuItem(_("What nozzle diameter do I have?"), &img::question_16x16) {
}

void MI_NOZZLE_DIAMETER_HELP::click(IWindowMenu &) {
    MsgBoxInfo(_("You can determine the nozzle diameter by counting the markings (dots) on the nozzle:\n"
                 "  0.40 mm nozzle: 3 dots\n"
                 "  0.60 mm nozzle: 4 dots\n\n"
                 "For more information, visit prusa.io/nozzle-types"),
        Responses_Ok);
}

ScreenPrinterSetup::ScreenPrinterSetup()
    : header(this, _("PRINTER SETUP"))
    , prompt(this, prompt_rect, is_multiline::yes, is_closed_on_click_t::no, _("Confirm your printer setup"))
    , menu(this, menu_rect) //
{
    ClrMenuTimeoutClose(); // don't close on menu timeout

    prompt.SetAlignment(Align_t::Center());
    prompt.SetTextColor(COLOR_BLACK);
    prompt.SetBackColor(COLOR_WHITE);

#if HAS_MINI_DISPLAY()
    prompt.set_font(Font::small);
#endif

    menu.menu.BindContainer(menu_container);
    CaptureNormalWindow(menu);
}
