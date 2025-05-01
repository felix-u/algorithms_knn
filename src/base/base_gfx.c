// TODO(felix): is vsync enabled by default? if not, let's enable it
// TODO(felix): get delta time

static Gfx_Font gfx_font_default_3x5 = {
    .width = 3, .height = 5,
    .glyphs = {
        [' '] = {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
        },
        ['!'] = {
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 0, 0,
            0, 1, 0,
        },
        ['"'] = {
            1, 0, 1,
            1, 0, 1,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
        },
        ['#'] = {
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
        },
        ['$'] = {
            0, 1, 0,
            0, 1, 1,
            0, 1, 0,
            1, 1, 0,
            0, 1, 0,
        },
        ['%'] = {
            1, 0, 1,
            0, 0, 1,
            0, 1, 0,
            1, 0, 0,
            1, 0, 1,
        },
        ['&'] = {
            0, 1, 0,
            1, 0, 1,
            1, 1, 0,
            1, 0, 1,
            0, 1, 1,
        },
        ['\''] = {
            0, 1, 0,
            0, 1, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
        },
        ['('] = {
            0, 1, 0,
            1, 0, 0,
            1, 0, 0,
            1, 0, 0,
            0, 1, 0,
        },
        [')'] = {
            0, 1, 0,
            0, 0, 1,
            0, 0, 1,
            0, 0, 1,
            0, 1, 0,
        },
        ['*'] = {
            0, 0, 0,
            1, 0, 1,
            0, 1, 0,
            1, 0, 1,
            0, 0, 0,
        },
        ['+'] = {
            0, 0, 0,
            0, 1, 0,
            1, 1, 1,
            0, 1, 0,
            0, 0, 0,
        },
        [','] = {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 1, 0,
            1, 0, 0,
        },
        ['-'] = {
            0, 0, 0,
            0, 0, 0,
            1, 1, 1,
            0, 0, 0,
            0, 0, 0,
        },
        ['.'] = {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 1, 0,
        },
        ['/'] = {
            0, 0, 1,
            0, 0, 1,
            0, 1, 0,
            1, 0, 0,
            1, 0, 0,
        },
        ['0'] = {
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['1'] = {
            0, 1, 0,
            1, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['2'] = {
            1, 1, 1,
            1, 0, 1,
            0, 0, 1,
            1, 1, 0,
            1, 1, 1,
        },
        ['3'] = {
            1, 1, 1,
            0, 0, 1,
            1, 1, 1,
            0, 0, 1,
            1, 1, 1,
        },
        ['4'] = {
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
            0, 0, 1,
            0, 0, 1,
        },
        ['5'] = {
            1, 1, 1,
            1, 0, 0,
            1, 1, 0,
            0, 0, 1,
            1, 1, 0,
        },
        ['6'] = {
            1, 1, 1,
            1, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['7'] = {
            1, 1, 1,
            0, 0, 1,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['8'] = {
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['9'] = {
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
            0, 0, 1,
            1, 1, 1,
        },
        [':'] = {
            0, 0, 0,
            0, 0, 0,
            0, 1, 0,
            0, 0, 0,
            0, 1, 0,
        },
        [';'] = {
            0, 0, 0,
            0, 1, 0,
            0, 0, 0,
            0, 1, 0,
            1, 0, 0,
        },
        ['<'] = {
            0, 0, 1,
            0, 1, 0,
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        },
        ['='] = {
            0, 0, 0,
            1, 1, 1,
            0, 0, 0,
            1, 1, 1,
            0, 0, 0,
        },
        ['>'] = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
            0, 1, 0,
            1, 0, 0,
        },
        ['?'] = {
            1, 1, 0,
            0, 0, 1,
            0, 1, 0,
            0, 0, 0,
            0, 1, 0,
        },
        ['@'] = {
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 0,
            1, 1, 1,
        },
        ['A'] = {
            0, 1, 0,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['B'] = {
            1, 1, 0,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 1, 0,
        },
        ['C'] = {
            0, 1, 1,
            1, 0, 0,
            1, 0, 0,
            1, 0, 0,
            0, 1, 1,
        },
        ['D'] = {
            1, 1, 0,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 0,
        },
        ['E'] = {
            1, 1, 1,
            1, 0, 0,
            1, 1, 1,
            1, 0, 0,
            1, 1, 1,
        },
        ['F'] = {
            1, 1, 1,
            1, 0, 0,
            1, 1, 1,
            1, 0, 0,
            1, 0, 0,
        },
        ['G'] = {
            0, 1, 1,
            1, 0, 0,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
        },
        ['H'] = {
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['I'] = {
            1, 1, 1,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            1, 1, 1,
        },
        ['J'] = {
            1, 1, 1,
            0, 0, 1,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
        },
        ['K'] = {
            1, 0, 1,
            1, 0, 1,
            1, 1, 0,
            1, 0, 1,
            1, 0, 1,
        },
        ['L'] = {
            1, 0, 0,
            1, 0, 0,
            1, 0, 0,
            1, 0, 0,
            1, 1, 1,
        },
        ['M'] = {
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['N'] = {
            1, 0, 1,
            1, 1, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['O'] = {
            0, 1, 0,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
        },
        ['P'] = {
            1, 1, 0,
            1, 0, 1,
            1, 1, 0,
            1, 0, 0,
            1, 0, 0,
        },
        ['Q'] = {
            0, 1, 0,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
            0, 0, 1,
        },
        ['R'] = {
            1, 1, 0,
            1, 0, 1,
            1, 1, 0,
            1, 0, 1,
            1, 0, 1,
        },
        ['S'] = {
            0, 1, 1,
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
            1, 1, 0,
        },
        ['T'] = {
            1, 1, 1,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['U'] = {
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['V'] = {
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
        },
        ['W'] = {
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
        },
        ['X'] = {
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
            1, 0, 1,
            1, 0, 1,
        },
        ['Y'] = {
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['Z'] = {
            1, 1, 1,
            0, 0, 1,
            0, 1, 0,
            1, 0, 0,
            1, 1, 1,
        },
        ['['] = {
            1, 1, 0,
            1, 0, 0,
            1, 0, 0,
            1, 0, 0,
            1, 1, 0,
        },
        ['\\'] = {
            1, 0, 0,
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
            0, 0, 1,
        },
        [']'] = {
            0, 1, 1,
            0, 0, 1,
            0, 0, 1,
            0, 0, 1,
            0, 1, 1,
        },
        ['^'] = {
            0, 1, 0,
            1, 0, 1,
            1, 0, 1,
            0, 0, 0,
            0, 0, 0,
        },
        ['_'] = {
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            1, 1, 1,
        },
        ['`'] = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
        },
        ['a'] = {
            0, 0, 0,
            0, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['b'] = {
            1, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['c'] = {
            0, 0, 0,
            1, 1, 1,
            1, 0, 0,
            1, 0, 0,
            1, 1, 1,
        },
        ['d'] = {
            0, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['e'] = {
            0, 0, 0,
            1, 1, 1,
            1, 1, 1,
            1, 0, 0,
            1, 1, 1,
        },
        ['f'] = {
            0, 1, 1,
            0, 1, 0,
            1, 1, 1,
            0, 1, 0,
            0, 1, 0,
        },
        ['g'] = {
            0, 0, 0,
            1, 1, 1,
            1, 1, 1,
            0, 0, 1,
            1, 1, 1,
        },
        ['h'] = {
            1, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['i'] = {
            0, 0, 0,
            1, 1, 0,
            0, 1, 0,
            0, 1, 0,
            1, 1, 1,
        },
        ['j'] = {
            0, 0, 0,
            0, 1, 1,
            0, 0, 1,
            1, 0, 1,
            0, 1, 1,
        },
        ['k'] = {
            1, 0, 0,
            1, 0, 1,
            1, 1, 0,
            1, 1, 0,
            1, 0, 1,
        },
        ['l'] = {
            1, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 1,
        },
        ['m'] = {
            0, 0, 0,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['n'] = {
            0, 0, 0,
            1, 1, 0,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
        },
        ['o'] = {
            0, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['p'] = {
            0, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 0,
        },
        ['q'] = {
            0, 0, 0,
            1, 1, 1,
            1, 0, 1,
            1, 1, 1,
            0, 0, 1,
        },
        ['r'] = {
            0, 0, 0,
            1, 1, 0,
            1, 0, 1,
            1, 0, 0,
            1, 0, 0,
        },
        ['s'] = {
            0, 0, 0,
            0, 1, 1,
            1, 1, 0,
            0, 0, 1,
            1, 1, 0,
        },
        ['t'] = {
            0, 1, 0,
            1, 1, 1,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['u'] = {
            0, 0, 0,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
        },
        ['v'] = {
            0, 0, 0,
            1, 0, 1,
            1, 0, 1,
            1, 0, 1,
            0, 1, 0,
        },
        ['w'] = {
            0, 0, 0,
            1, 0, 1,
            1, 0, 1,
            1, 1, 1,
            1, 0, 1,
        },
        ['x'] = {
            0, 0, 0,
            1, 0, 1,
            0, 1, 0,
            0, 1, 0,
            1, 0, 1,
        },
        ['y'] = {
            0, 0, 0,
            1, 0, 1,
            1, 1, 1,
            0, 0, 1,
            1, 1, 0,
        },
        ['z'] = {
            0, 0, 0,
            1, 1, 1,
            0, 1, 1,
            1, 0, 0,
            1, 1, 1,
        },
        ['{'] = {
            0, 1, 1,
            0, 1, 0,
            1, 0, 0,
            0, 1, 0,
            0, 1, 1,
        },
        ['|'] = {
            0, 1, 0,
            0, 1, 0,
            0, 0, 0,
            0, 1, 0,
            0, 1, 0,
        },
        ['}'] = {
            1, 1, 0,
            0, 1, 0,
            0, 0, 1,
            0, 1, 0,
            1, 1, 0,
        },
        ['~'] = {
            0, 0, 0,
            1, 0, 0,
            1, 1, 1,
            0, 0, 1,
            0, 0, 0,
        },
    },
};

static V2 gfx_get_real_screen_size(Gfx_Render_Context *gfx) {
    gfx->frame_info.monitor_handle = MonitorFromWindow(gfx->window_handle, MONITOR_DEFAULTTONEAREST);

    MONITORINFO info = { .cbSize = sizeof(MONITORINFO) };
    bool ok = GetMonitorInfoA(gfx->frame_info.monitor_handle, &info);
    assert(ok);

    V2 size;
    size.x = (f32)(info.rcMonitor.right - info.rcMonitor.left);
    size.y = (f32)(info.rcMonitor.bottom - info.rcMonitor.top);
    return size;
}

static inline u32 gfx_rgba8_from_v4(V4 v) {
    u32 result =
        ((u32)(v.r * 255.f + 0.5f) << 24) |
        ((u32)(v.g * 255.f + 0.5f) << 16) |
        ((u32)(v.b * 255.f + 0.5f) << 8) |
        ((u32)(v.a * 255.f + 0.5f));
    return result;
}

static void gfx_clear(Gfx_Render_Context *gfx, u32 rgb) {
    // TODO(felix): memcpy once first row is filled
    u32 width = (u32)gfx->frame_info.virtual_window_size.x;
    u32 height = (u32)gfx->frame_info.virtual_window_size.y;
    for (u32 y = 0; y < height; y += 1) {
        u32 *row = gfx->pixels + y * width;
        for (u32 x = 0; x < width; x += 1) {
            u32 *pixel = row + x;
            *pixel = rgb;
        }
    }
}

static void gfx_draw_line(Gfx_Render_Context *gfx, V2 start, V2 end, f32 thickness, u32 rgb) {
    // Reference: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    // TODO(felix)
    assert(thickness == 1.f);

    i32 start_x = (i32)(start.x + 0.5f);
    i32 end_x = (i32)(end.x + 0.5f);

    i32 start_y = (i32)(start.y + 0.5f);
    i32 end_y = (i32)(end.y + 0.5f);

    bool is_steep = abs(end_y - start_y) > abs(end_x - start_x);
    if (is_steep) {
        swap(i32, &start_x, &start_y);
        swap(i32, &end_x, &end_y);
    }

    bool right_to_left = start_x > end_x;
    if (right_to_left) {
        swap(i32, &start_x, &end_x);
        swap(i32, &start_y, &end_y);
    }

    i32 distance_x = end_x - start_x;
    i32 distance_y = abs(end_y - start_y);
    i32 error = distance_x / 2;
    i32 y_step = start_y < end_y ? 1 : -1;

    i32 width = (i32)gfx->frame_info.virtual_window_size.x;
    i32 height = (i32)gfx->frame_info.virtual_window_size.y;

    i32 y = start_y;
    for (i32 x = start_x; x < end_x; x += 1) {
        if (is_steep) {
            if (y < 0 || y >= width) continue;
            if (x < 0 || x >= height) continue;
            gfx_set_pixel(gfx, y, x, rgb);
        } else {
            if (x < 0 || x >= width) continue;
            if (y < 0 || y >= height) continue;
            gfx_set_pixel(gfx, x, y, rgb);
        }

        error -= distance_y;
        if (error < 0) {
            y += y_step;
            error += distance_x;
        }
    }
}

static inline void gfx_set_pixel(Gfx_Render_Context *gfx, i32 x, i32 y, u32 rgb) {
    // TODO(felix): alpha blending (probably should have two set_pixel functions)

    i32 width = (i32)gfx->frame_info.virtual_window_size.x;
    i32 height = (i32)gfx->frame_info.virtual_window_size.y;

    assert(x >= 0);
    assert(x < width);

    assert(y >= 0);
    assert(y < height);

    gfx->pixels[y * width + x] = rgb;
}

static void gfx_draw_rectangle(Gfx_Render_Context *gfx, i32 top, i32 left, i32 width, i32 height, u32 rgb) {
    i32 virtual_width = (i32)gfx->frame_info.virtual_window_size.x;
    i32 virtual_height = (i32)gfx->frame_info.virtual_window_size.y;

    i32 bottom = clamp(top + height, 0, virtual_height);
    i32 right = clamp(left + width, 0, virtual_width);
    top = clamp(top, 0, virtual_height - 1);
    left = clamp(left, 0, virtual_width - 1);

    // TODO(felix): can optimise by doing memcpy after first row
    for (i32 y = top; y < bottom; y += 1) {
        for (i32 x = left; x < right; x += 1) {
            gfx_set_pixel(gfx, x, y, rgb);
        }
    }
}

static void gfx_draw_text(Gfx_Render_Context *gfx, Gfx_Font font, String string, i32 x, i32 y, u32 rgb) {
    i32 top = y;
    i32 left = x;

    // TODO(felix): make customisable
    i32 tracking = 1;
    i32 step = font.width + tracking;

    assert(string.count > 0);
    for (usize i = 0; i < string.count; i += 1, left += step) {
        u8 character = string.data[i];
        assert(character < 128);

        u8 *glyph = font.glyphs[character];
        for (i32 font_y = 0; font_y < font.height; font_y += 1) {
            y = top + font_y;
            u8 *glyph_row = &glyph[font_y * font.width];

            for (i32 font_x = 0; font_x < font.width; font_x += 1) {
                u8 glyph_value = *(glyph_row + font_x);
                if (glyph_value == 0) continue;

                x = left + font_x;
                gfx_set_pixel(gfx, x, y, rgb);
            }
        }
    }
}

static Gfx_Render_Context *gfx_win32_window_procedure_context__;
static LRESULT gfx_win32_window_procedure(HWND window, u32 message, WPARAM w, LPARAM l) {
    Gfx_Render_Context *gfx = gfx_win32_window_procedure_context__;
    switch (message) {
        case WM_CLOSE: DestroyWindow(window); break;
        case WM_DESTROY: PostQuitMessage(0); break;
        case WM_DISPLAYCHANGE: gfx->frame_info.real_screen_size = gfx_get_real_screen_size(gfx); break;
        case WM_DPICHANGED: {
            f32 dpi_1x = 96.f;
            gfx->frame_info.dpi_scale = (f32)GetDpiForWindow(window) / dpi_1x;
        } break;
        case WM_GETMINMAXINFO: { // TODO(felix): not working. The goal is to not allow the window to become smaller than a certain size
            MINMAXINFO *info = (MINMAXINFO *)l;
            int min_width = 100, min_height = 100;
            info->ptMinTrackSize.x = min_width;
            info->ptMinTrackSize.y = min_height;
        } break;
        case WM_KEYDOWN: {
            gfx->frame_info.key_is_up[w] = false;
            gfx->frame_info.key_is_down[w] = true;
        } break;
        case WM_KEYUP: {
            gfx->frame_info.key_is_down[w] = false;
            gfx->frame_info.key_is_up[w] = true;
        } break;
        case WM_LBUTTONDOWN: gfx->frame_info.mouse_left_clicked = true; gfx->frame_info.mouse_left_down = true; break;
        case WM_LBUTTONUP: gfx->frame_info.mouse_left_clicked = false; gfx->frame_info.mouse_left_down = false; break;
        case WM_MOUSEMOVE: {
            // NOTE(felix): we include windowsx.h just for these two lparam macros. Can we remove that and do something else?
            gfx->frame_info.real_mouse_position.x = (f32)GET_X_LPARAM(l);
            gfx->frame_info.real_mouse_position.y = (f32)GET_Y_LPARAM(l);
        } break;
        case WM_RBUTTONDOWN: gfx->frame_info.mouse_right_clicked = true; gfx->frame_info.mouse_right_down = true; break;
        case WM_RBUTTONUP: gfx->frame_info.mouse_right_clicked = false; gfx->frame_info.mouse_right_down = false; break;
        case WM_SIZE: {
            gfx->frame_info.real_window_size.x = (f32)LOWORD(l);
            gfx->frame_info.real_window_size.y = (f32)HIWORD(l);
        } break;
        case WM_SIZING: {
            // TODO(felix): handle redrawing here so that we're not blocked while resizing
            // NOTE(felix): apparently there might be some problems with this and the better way to do it is for the thread with window events to be different than the application thread that submits draw calls and updates the program
        } break;
    }
    return DefWindowProcA(window, message, w, l);
}

static Gfx_Render_Context gfx_window_create(Arena *arena, char *window_name, u32 width, u32 height) {
    Gfx_Render_Context gfx = {0};
    gfx_win32_window_procedure_context__ = &gfx;

    // window
    {
        WNDCLASSA window_class = {
            .lpfnWndProc = gfx_win32_window_procedure,
            .hCursor = LoadCursorA(0, IDC_ARROW),
            .lpszClassName = window_name,
        };
        ATOM atom_result = RegisterClassA(&window_class);
        ensure(atom_result != 0);

        bool ok = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        ensure(ok);

        DWORD extended_style = 0;
        char *class_name = window_name;
        int x = CW_USEDEFAULT, y = CW_USEDEFAULT, window_width = CW_USEDEFAULT, window_height = CW_USEDEFAULT;
        gfx.window_handle = CreateWindowExA(extended_style, class_name, window_name, WS_OVERLAPPEDWINDOW, x, y, window_width, window_height, 0, 0, 0, 0);
        ensure(gfx.window_handle != 0);
    }

    HRESULT hresult = ShowWindow(gfx.window_handle, SW_SHOWDEFAULT);
    ensure(hresult == S_OK);

    // TODO(felix): option to have resizable *virtual* window, not just resizable real window
    gfx.frame_info.virtual_window_size = (V2){ .x = (f32)width, .y = (f32)height };
    gfx.pixels = arena_make(arena, width * height, sizeof(u32));

    HDC window_device_context = GetDC(gfx.window_handle);
    {
        i32 virtual_width = (i32)gfx.frame_info.virtual_window_size.x;
        i32 virtual_height = (i32)gfx.frame_info.virtual_window_size.y;

        BITMAPINFOHEADER bitmap_info_header = {
            .biSize = sizeof(bitmap_info_header),
            .biWidth = virtual_width,
            .biHeight = -virtual_height,
            .biPlanes = 1,
            .biBitCount = sizeof(*(gfx.pixels)) * 8,
        };

        gfx.memory_device_context = CreateCompatibleDC(window_device_context);
        ensure(gfx.memory_device_context != 0);

        gfx.bitmap = CreateDIBSection(gfx.memory_device_context, (BITMAPINFO *)&bitmap_info_header, DIB_RGB_COLORS, (void **)&gfx.pixels, 0, 0);
        ensure(gfx.bitmap != 0);

        SelectObject(gfx.memory_device_context, gfx.bitmap);
    }
    ReleaseDC(gfx.window_handle, window_device_context);

    return gfx;
}

static bool gfx_window_should_close(Gfx_Render_Context *gfx) {
    memcpy(gfx->frame_info.key_was_down_last_frame, gfx->frame_info.key_is_down, sizeof(gfx->frame_info.key_is_down));
    memcpy(gfx->frame_info.key_was_up_last_frame, gfx->frame_info.key_is_up, sizeof(gfx->frame_info.key_is_up));
    gfx->frame_info.mouse_left_clicked = false;
    gfx->frame_info.mouse_right_clicked = false;

    gfx_win32_window_procedure_context__ = gfx;
    for (MSG message = {0}; PeekMessageA(&message, 0, 0, 0, PM_REMOVE);) {
        if (message.message == WM_QUIT) {
            DeleteObject(gfx->bitmap);
            DeleteDC(gfx->memory_device_context);
            return true;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    // TODO(felix): vsync? or more likely, """vsync"""

    HDC window_device_context = GetDC(gfx->window_handle);
    {
        i32 virtual_width = (i32)gfx->frame_info.virtual_window_size.x;
        i32 virtual_height = (i32)gfx->frame_info.virtual_window_size.y;
        i32 real_width = (i32)gfx->frame_info.real_window_size.x;
        i32 real_height = (i32)gfx->frame_info.real_window_size.y;

        StretchBlt(
            window_device_context,
            0, 0, real_width, real_height,
            gfx->memory_device_context,
            0, 0, virtual_width, virtual_height,
            SRCCOPY
        );
    }
    ReleaseDC(gfx->window_handle, window_device_context);

    return false;
}
