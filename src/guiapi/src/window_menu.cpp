// window_menu.cpp
#include "window_menu.h" //C compatible, todo remove
#include "window_menu.hpp"
#include "gui.h"
#include "cmath_ext.h"
#include "sound_C_wrapper.h"

#define WIO_MIN  0
#define WIO_MAX  1
#define WIO_STEP 2

/*****************************************************************************/
//WindowMenuItem
WindowMenuItem::WindowMenuItem(uint16_t type, const char *text, uint16_t id_icon)
    : type(type)
    , id_icon(id_icon) {
    //strncpy(this->label.data, label, this->label.size);
}

//ctor without param creates LABEL
WindowMenuItem::WindowMenuItem(const char *text, uint16_t id_icon, uint16_t flags)
    : WindowMenuItem(flags, text, id_icon) {
}

WindowMenuItem::WindowMenuItem(WI_SPIN_t wi_spin, const char *text, uint16_t id_icon)
    : WindowMenuItem(WI_SPIN, text, id_icon) {
    data = wi_spin;
}

WindowMenuItem::WindowMenuItem(WI_SPIN_FL_t wi_spin_fl, const char *text, uint16_t id_icon)
    : WindowMenuItem(WI_SPIN_FL, text, id_icon) {
    data = wi_spin_fl;
}

WindowMenuItem::WindowMenuItem(WI_SWITCH_t wi_switch, const char *text, uint16_t id_icon, bool switch_not_select)
    : WindowMenuItem(WI_SWITCH, text, id_icon) {
    data = wi_switch;
}

WindowMenuItem::WindowMenuItem(WI_SELECT_t wi_select, const char *text, uint16_t id_icon, bool switch_not_select)
    : WindowMenuItem(WI_SELECT, text, id_icon) {
    data = wi_select;
}

void window_menu_inc(window_menu_t *window, int dif);
void window_menu_dec(window_menu_t *window, int dif);
void window_menu_item_spin(window_menu_t *window, int dif);
void window_menu_item_spin_fl(window_menu_t *window, int dif);
void window_menu_item_switch(window_menu_t *window);
void window_menu_item_select(window_menu_t *window, int dif);

WindowMenuItem undefined = { "No menu_items fce!", 0, WI_LABEL | WI_DISABLED };

void window_menu_items(window_menu_t *pwindow_menu, uint16_t index,
    WindowMenuItem **ppitem, void *data) {
    *ppitem = &undefined;
}

void window_menu_init(window_menu_t *window) {
    window->color_back = gui_defaults.color_back;
    window->color_text = gui_defaults.color_text;
    window->color_disabled = gui_defaults.color_disabled;
    window->font = gui_defaults.font;
    window->padding = gui_defaults.padding;
    window->icon_rect = rect_ui16(0, 0, 16, 16);
    window->alignment = gui_defaults.alignment;
    window->count = 0;
    window->index = 0;
    window->top_index = 0;
    window->mode = 0;
    window->menu_items = window_menu_items;
    window->data = NULL;
    window->win.flg |= WINDOW_FLG_ENABLED;
}

void window_menu_done(window_menu_t *window) {
}

void window_menu_calculate_spin(WI_SPIN_t *item, char *value) {
    const char *format;

    if (item->range[WIO_STEP] < 10)
        format = "%.3f";
    else if (item->range[WIO_STEP] < 100)
        format = "%.2f";
    else if (item->range[WIO_STEP] < 1000)
        format = "%.1f";
    else
        format = "%.f";
    sprintf(value, format, item->value * 0.001);
}

void window_menu_set_item_index(window_t *window, int index) {
    if (window->cls->cls_id == WINDOW_CLS_MENU) {
        if (((window_menu_t *)window)->count > index) {
            ((window_menu_t *)window)->index = index;
        }
    }
}

void _window_menu_draw_value(window_menu_t *window, const char *value,
    rect_ui16_t *p_rc, color_t color_text_option, color_t color_back) {
    rect_ui16_t vrc = {
        uint16_t(p_rc->x + p_rc->w), p_rc->y, uint16_t(window->font->w * strlen(value) + window->padding.left + window->padding.right), p_rc->h
    };
    vrc.x -= vrc.w;
    p_rc->w -= vrc.w;

    render_text_align(vrc, value, window->font,
        color_back, color_text_option, window->padding, window->alignment);
}

void window_menu_draw(window_menu_t *window) {
    if (!((window->win.flg & (WINDOW_FLG_INVALID | WINDOW_FLG_VISIBLE)) == (WINDOW_FLG_INVALID | WINDOW_FLG_VISIBLE))) {
        return;
    }

    int item_height = window->font->h + window->padding.top + window->padding.bottom;
    rect_ui16_t rc_win = window->win.rect;

    int visible_count = rc_win.h / item_height;
    int i;
    for (i = 0; i < visible_count && i < window->count; i++) {
        int idx = i + window->top_index;
        WindowMenuItem *item;
        window->menu_items(window, idx, &item, window->data);

        color_t color_text = window->color_text;
        color_t color_back = window->color_back;
        uint8_t swap = 0;

        rect_ui16_t rc = { rc_win.x, uint16_t(rc_win.y + i * item_height),
            rc_win.w, uint16_t(item_height) };
        padding_ui8_t padding = window->padding;

        if (rect_in_rect_ui16(rc, rc_win)) {
            if (!item->IsEnabled()) {
                color_text = window->color_disabled;
            }

            if ((window->win.flg & WINDOW_FLG_FOCUSED) && (window->index == idx)) {
                color_t swp = color_text;
                color_text = color_back;
                color_back = swp;
                swap = ROPFN_SWAPBW;
            }

            color_t color_option = color_text;
            if (window->mode && swap) {
                color_option = COLOR_ORANGE;
            }

            switch (item->type & 0xff) //& 0xff == remove flags
            {
            case WI_SPIN:
            case WI_SPIN_FL: {
                char value[20] = { '\0' };
                if (item->type & WI_SPIN_FL)
                    sprintf(value, item->data.wi_spin_fl.prt_format, (double)item->data.wi_spin_fl.value);
                else
                    window_menu_calculate_spin(&(item->data.wi_spin), value);

                _window_menu_draw_value(window, value, &rc, color_option, color_back);
            } break;
            case WI_SWITCH:
                if (swap)
                    color_option = COLOR_ORANGE;
            case WI_SELECT: {
                const char *value = ((const char **)item->data.wi_select.strings)[item->data.wi_select.index];

                _window_menu_draw_value(window, value, &rc, color_option, color_back);
            } break;
            }

            if (item->id_icon) {
                rect_ui16_t irc = { rc.x, rc.y,
                    window->icon_rect.w, window->icon_rect.h };
                rc.x += irc.w;
                rc.w -= irc.w;
                render_icon_align(irc, item->id_icon,
                    window->color_back, RENDER_FLG(ALIGN_CENTER, swap));
            } else {
                padding.left += window->icon_rect.w;
            }

            // render
            render_text_align(rc, item->label.data(), window->font,
                color_back, color_text,
                padding, window->alignment);
        }
    }
    rc_win.h = rc_win.h - (i * item_height);

    if (rc_win.h) {
        rc_win.y += i * item_height;
        display->fill_rect(rc_win, window->color_back);
    }
}

void window_menu_event(window_menu_t *window, uint8_t event, void *param) {
    window->src_event = event;
    window->src_param = param;
    switch (event) {
    case WINDOW_EVENT_BTN_DN:
        if (window->mode != WI_LABEL) {
            window->mode = WI_LABEL;
            screen_dispatch_event(NULL, WINDOW_EVENT_CHANGE, (void *)window->index);
        } else {
            WindowMenuItem *item;
            window->menu_items(window, window->index, &item, window->data);

            //mask all flags but WI_DISABLED
            if ((item->IsEnabled())) {
                //"& 0xff" == mask all flags off
                //switch does not set type, i is acting like label i suppose
                if ((item->type & 0xff) == WI_SWITCH) {
                    window_menu_item_switch(window);
                } else {
                    window->mode = item->type & 0xff;
                }
            } else {
                screen_dispatch_event(NULL, WINDOW_EVENT_CLICK, (void *)window->index);
                return;
            }
            screen_dispatch_event(NULL, WINDOW_EVENT_CLICK, (void *)window->index);
        }
        _window_invalidate((window_t *)window);
        break;
    case WINDOW_EVENT_ENC_DN:
        window_menu_dec(window, (int)param);
        if (window->mode != WI_LABEL) {
            screen_dispatch_event(NULL, WINDOW_EVENT_CHANGING, (void *)window->index);
        }
        break;
    case WINDOW_EVENT_ENC_UP:
        window_menu_inc(window, (int)param);
        if (window->mode != WI_LABEL) {
            screen_dispatch_event(NULL, WINDOW_EVENT_CHANGING, (void *)window->index);
        }
        break;
    case WINDOW_EVENT_CAPT_1:
        //TODO: change flag to checked
        break;
    }
}

void window_menu_inc(window_menu_t *window, int dif) {
    switch (window->mode) {
    case WI_SPIN:
        window_menu_item_spin(window, dif);
        break;
    case WI_SPIN_FL:
        window_menu_item_spin_fl(window, dif);
        break;
    case WI_SELECT:
        window_menu_item_select(window, dif);
        break;
    default: {
        // WI_LABEL
        //all items can be in label mode
        int item_height = window->font->h + window->padding.top + window->padding.bottom;
        int visible_count = window->win.rect.h / item_height;
        int old = window->index;
        window->index += dif;
        // play sound at first or last index of menu
        if (window->index < 0) {
            window->index = 0;
            Sound_Play(eSOUND_TYPE_BlindAlert);
        }
        if (window->index >= window->count) {
            window->index = window->count - 1;
            Sound_Play(eSOUND_TYPE_BlindAlert);
        }

        if (window->index < window->top_index)
            window->top_index = window->index;
        if (window->index >= (window->top_index + visible_count))
            window->top_index = window->index - visible_count + 1;

        if (window->index != old) { // optimalization do not redraw when no change - still on end
            _window_invalidate((window_t *)window);
        }
    }
    }
}

void window_menu_dec(window_menu_t *window, int dif) {
    window_menu_inc(window, -dif);
}

const window_class_menu_t window_class_menu = {
    {
        WINDOW_CLS_MENU,
        sizeof(window_menu_t),
        (window_init_t *)window_menu_init,
        (window_done_t *)window_menu_done,
        (window_draw_t *)window_menu_draw,
        (window_event_t *)window_menu_event,
    },
};

void window_menu_item_spin(window_menu_t *window, int dif) {
    WindowMenuItem *item;
    window->menu_items(window, window->index, &item, window->data);

    const int32_t *range = item->data.wi_spin.range;
    int32_t old = item->data.wi_spin.value;

    if (dif > 0) {
        item->data.wi_spin.value = MIN(item->data.wi_spin.value + dif * range[WIO_STEP], range[WIO_MAX]);
    } else {
        item->data.wi_spin.value = MAX(item->data.wi_spin.value + dif * range[WIO_STEP], range[WIO_MIN]);
    }

    if (old != item->data.wi_spin.value)
        _window_invalidate((window_t *)window);
}

void window_menu_item_spin_fl(window_menu_t *window, int dif) {
    WindowMenuItem *item;
    window->menu_items(window, window->index, &item, window->data);

    const float *range = item->data.wi_spin_fl.range;
    float old = item->data.wi_spin_fl.value;

    if (dif > 0) {
        item->data.wi_spin_fl.value = MIN(item->data.wi_spin_fl.value + (float)dif * range[WIO_STEP], range[WIO_MAX]);
    } else {
        item->data.wi_spin_fl.value = MAX(item->data.wi_spin_fl.value + (float)dif * range[WIO_STEP], range[WIO_MIN]);
    }

    if (old != item->data.wi_spin_fl.value)
        _window_invalidate((window_t *)window);
}

void window_menu_item_switch(window_menu_t *window) {
    WindowMenuItem *item;
    window->menu_items(window, window->index, &item, window->data);

    const char **strings = item->data.wi_switch.strings;
    size_t size = 0;
    while (strings[size] != NULL) {
        size++;
    }
    item->data.wi_switch.index++;
    if (item->data.wi_switch.index >= size) {
        item->data.wi_switch.index = 0;
    }
}

void window_menu_item_select(window_menu_t *window, int dif) {
    WindowMenuItem *item;
    window->menu_items(window, window->index, &item, window->data);

    const char **strings = item->data.wi_select.strings;
    size_t size = 0;
    while (strings[size] != NULL) {
        size++;
    }

    if (dif > 0) {
        item->data.wi_select.index++;
        if (item->data.wi_select.index >= size) {
            item->data.wi_select.index = 0;
        }
    } else {
        item->data.wi_select.index--;
        if (item->data.wi_select.index < 0) {
            item->data.wi_select.index = size - 1;
        }
    }
    _window_invalidate((window_t *)window);
}

void WindowMenuItem::Change(int dif) {
}

void WI_LABEL_t::Change(int dif) {
}

void WI_SPIN_t::Change(int dif) {
}

void WI_SPIN_FL_t::Change(int dif) {
}

void WI_SWITCH_t::Change(int dif) {
}

void WI_SELECT_t::Change(int dif) {
}
