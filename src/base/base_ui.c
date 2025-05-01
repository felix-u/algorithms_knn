// NOTE(felix): How this needs to look:
// 1. Build box hierarchy. Use last frame's data (looked up by hash) for input
//  requires hash links { UI_Box *hash_prev, *hash_next }
//  requires keying and frame info { UI_Key key; usize last_frame_touched_idx }
//      a) look up in box hash table to be able to use previous frame's layout to compute input
//      b) iterate through hash table. if a box's last_frame_touched_idx < current_frame_idx, remove
// 2. Layout pass (compute sizes)
//  requires tree links { UI_Box *parent, *first, *last, *prev, *next }
//      a) calculate standalone sizes, which do not depend on other boxs
//      b) pre-order: calculate sizes dependent on ancestor sizes
//      c) post-order: calculate sizes dependent on descendant sizes
//      d) pre-order: solve violations using strictness value
//      e) pre-order: using calculated sizes, compute relative positions of each box (lay out on axis specified by parent),
//         compute final screen coordinate rectangle
// 3. Render

static UI_Box *ui_box_map_memory[ui_box_map_size];

static Array_UI_Box_Pointer ui_box_hashmap = {
    .data = ui_box_map_memory,
    .capacity = ui_box_map_size,
};

static UI_Box *ui_border_box(UI *ui) {
    UI_Box *panel = ui_box_frame_local_not_keyed(ui);
    panel->build.flags = ui_box_flag_draw_border;
    for_ui_axis (axis) panel->build.size[axis].kind = ui_size_kind_sum_of_children;
    return panel;
}

static UI_Box *ui_box_argument_struct(UI_Box_Arguments arguments) {
    UI *ui = arguments.ui;
    String string = arguments.string;

    assert(string.count != 0 && string.data != 0);

    String display_string = string;
    V2 display_string_size = {0};
    String hash_string = string;
    if (arguments.only_hash) {
        display_string = (String){0};
        #if BUILD_DEBUG
            for (usize i = 0; i < string.count; i += 1) {
                if (string.data[i] != '#') continue;
                i += 1;
                if (i == string.count || string.data[i] != '[') continue;
                panic("you called `ui_box` with `.only_hash = true`, but the string `%` has a display/hash specifier at index %", fmt(String, string), fmt(usize, i));
            }
        #endif
    } else {
        usize specifier_start_index = 0, specifier_end_index = 0;
        enum { before_specifier, after_specifier, specifier_position_count };
        bool should_hash[specifier_position_count] = {0};
        bool should_display[specifier_position_count] = {0};

        bool there_is_a_specifier = false;
        for (usize i = 0; i < string.count; i += 1) {
            if (string.data[i] != '#') continue;

            i += 1;
            if (i == string.count) break;
            if (string.data[i] != '[') continue;

            there_is_a_specifier = true;

            specifier_start_index = i - 1;
            usize position = before_specifier;

            for (; i < string.count; i += 1) switch (string.data[i]) {
                case ']': specifier_end_index = i; goto compute_specifier;
                case 'h': should_hash[position] = true; break;
                case 'd': should_display[position] = true; break;
                case ',': {
                    position += 1;
                    if (position != after_specifier) panic("specifier in string '%' has too many commas", fmt(String, string));
                } break;
                default: break;
            }
            panic("specifier in string '%' never closed with ']'", fmt(String, string));
        }
        goto done_parsing;

        compute_specifier: {
            assert(specifier_end_index != 0 || should_hash[before_specifier] || should_hash[after_specifier]);
            String string_before_specifier = string_range(string, 0, specifier_start_index);
            String string_after_specifier = string_range(string, specifier_end_index + 1, string.count);

            Array_u8 hash_string_builder = { .arena = ui->frame_arena };
            array_ensure_capacity(&hash_string_builder, string.count);
            if (should_hash[before_specifier]) array_push_slice_assume_capacity(&hash_string_builder, &string_before_specifier);
            if (should_hash[after_specifier]) array_push_slice_assume_capacity(&hash_string_builder, &string_after_specifier);

            Array_u8 display_string_builder = { .arena = ui->frame_arena };
            array_ensure_capacity(&display_string_builder, string.count);
            if (should_display[before_specifier]) array_push_slice_assume_capacity(&display_string_builder, &string_before_specifier);
            if (should_display[after_specifier]) array_push_slice_assume_capacity(&display_string_builder, &string_after_specifier);

            hash_string = bit_cast(String) hash_string_builder;
            display_string = bit_cast(String) display_string_builder;
        }

        done_parsing: {
            // NOTE(felix): there's probably a case to make for allowing empty display strings
            assert(display_string.count != 0);
            #if BUILD_DEBUG
                if (there_is_a_specifier) for (usize position = 0; position < specifier_position_count; position += 1) {
                    assert(should_hash[position] || should_display[position]);
                }
            #else
                discard(there_is_a_specifier);
            #endif

            // TODO(felix): support newlines and tab characters
            u8 tracking = 1; // TODO(felix): configurable in style
            display_string_size = (V2){
                .x = (f32)(display_string.count * (ui->gfx->font.width + tracking)),
                .y = (f32)(ui->gfx->font.height),
            };
        }
    }

    assert(hash_string.count != 0);

    // djb2 hash
    // TODO(felix): add hashmap to base and use
    usize key = 5381;
    for (usize i = 0; i < hash_string.count; i += 1) {
        key = ((key << 5) + key) + hash_string.data[i];
    }
    key %= ui->box_hashmap.capacity;

    for (UI_Box *match = ui->box_hashmap.data[key]; match != 0; match = match->next) {
        if (!string_equal(hash_string, match->build.hash_string)) continue;
        // TODO(felix): there are fields we need to initialise here that are currently only initialised when allocating a new widget
        // TODO(felix): also, I think, fields that need to be zeroed
        // TODO(felix): need to do some cleanup here to unify control flow
        match->build.display_string = arena_push(ui->frame_arena, display_string);
        if (display_string.count != 0) {
            match->computed_size_px[ui_axis_x] = display_string_size.x + match->style.pad.x * 2.f;
            match->computed_size_px[ui_axis_y] = display_string_size.y + match->style.pad.y * 2.f;
        }
        return match;
    }

    // TODO(felix): check freelist in ui_state before allocating new memory. To populate said freelist, we'll need to prune the hashmap every frame.
    UI_Box *new = arena_make(ui->persistent_arena, 1, sizeof(UI_Box));
    ui->box_hashmap.data[key] = new;

    // TODO(felix): I think the new style should be applied even above, in the case where the widget already exists
    UI_Box_Style target_style = {
        .pad = slice_get_last_assume_not_empty(ui->style_stack.pad),
        .margin = slice_get_last_assume_not_empty(ui->style_stack.margin),
        .clr_fg = slice_get_last_assume_not_empty(ui->style_stack.clr_fg),
        .clr_bg = slice_get_last_assume_not_empty(ui->style_stack.clr_bg),
        .clr_border = slice_get_last_assume_not_empty(ui->style_stack.clr_border),
    };
    *new = (UI_Box){
        .key = key,
        .target_style = target_style,
        .computed_size_px = { display_string_size.x + target_style.pad.x * 2.f, display_string_size.y + target_style.pad.y * 2.f },
    };
    new->build.hash_string = arena_push(ui->persistent_arena, hash_string);
    new->build.display_string = arena_push(ui->frame_arena, display_string);
    return new;
}

// TODO(felix): can take from freelist here also once implemented
static UI_Box *ui_box_frame_local_not_keyed(UI *ui) {
    UI_Box *box = arena_make(ui->frame_arena, 1, sizeof(UI_Box));
    return box;
}

// NOTE(felix): Since a root node is guaranteed before user UI code, a possible optimisation/simplification could be to remove
// all checks for nonzero root and current_parent throughout UI functions
static void ui_begin_build(UI *ui) {
    ui->current_parent = 0;
    ui->root = 0;

    f32 dpi_scale = ui->gfx->frame_info.dpi_scale;

    // TODO(felix): this should probably push the default style from a style struct, so that usage code can also do ui_push_style(custom_style)
    ui->style_stack.pad.count = 0;
    V2 default_pad = v2_scale((V2){ .x = 5.f, .y = 2.5f }, dpi_scale);
    array_push(&ui->style_stack.pad, &default_pad);

    ui->style_stack.margin.count = 0;
    V2 default_margin = v2_scale((V2){ .x = 5.f, .y = 5.f }, dpi_scale);
    array_push(&ui->style_stack.margin, &default_margin);

    ui->style_stack.clr_fg.count = 0;
    array_push(&ui->style_stack.clr_fg, (&(V4){ .r = 1.f, .g = 0, .b = 0, .a = 1.f }));

    ui->style_stack.clr_bg.count = 0;
    array_push(&ui->style_stack.clr_bg, &(V4){0});

    ui->style_stack.clr_border.count = 0;
    array_push(&ui->style_stack.clr_border, &(V4){ .a = 0.7f });

    ui_push_parent(ui, ui_row(ui));
}

static void ui_end_build(UI *ui) {
    ui_compute_layout_standalone(ui, ui->root);
    ui_compute_layout_dependent_ancestor(ui, ui->root);
    ui_compute_layout_dependent_descendant(ui, ui->root);
    ui_compute_layout_solve_violations(ui, ui->root);
    ui_compute_layout_relative_positions_and_rect(ui, ui->root);
}

static UI_Interaction ui_button(UI *ui, char *format, ...) {
    va_list args;
    va_start(args, format);
    String string = string_vprint(ui->frame_arena, format, args);
    va_end(args);

    UI_Box *button = ui_box(ui, string);
    button->build.flags = ui_box_flag_clickable | ui_box_flag_draw_text | ui_box_flag_draw_border | ui_box_flag_draw_background;
    for_ui_axis (axis) button->build.size[axis].kind = ui_size_kind_text;
    // button->target_style = (UI_Box_Style){
    //     .clr_border = { .a = 0.7f },
    // };
    return ui_push(ui, button);
}

static UI_Box *ui_column(UI *ui) {
    UI_Box *column = ui_box_frame_local_not_keyed(ui);
    column->build.flags = ui_axis_y;
    column->build.size[ui_axis_x].kind = ui_size_kind_largest_child;
    column->build.size[ui_axis_y].kind = ui_size_kind_sum_of_children;
    return column;
}

static void ui_compute_layout_dependent_ancestor(UI *ui, UI_Box *box) {
    discard(ui);
    discard(box);
    // TODO(felix)
    // NOTE(felix): preorder
}

static void ui_compute_layout_dependent_descendant(UI *ui, UI_Box *box) {
    if (box == 0) return;
    if (box->first == 0) return;

    for (UI_Box *child = box->first; child != 0; child = child->next) ui_compute_layout_dependent_descendant(ui, child);

    for_ui_axis (axis) switch (box->build.size[axis].kind) {
        case ui_size_kind_sum_of_children: {
            box->computed_size_px[axis] = 0;
            for (UI_Box *child = box->first; child != 0; child = child->next) {
                box->computed_size_px[axis] += child->computed_size_px[axis];
                box->computed_size_px[axis] += child->style.margin.items[axis];
            }
            box->computed_size_px[axis] += box->last->style.margin.items[axis];
        } break;
        case ui_size_kind_largest_child: {
            box->computed_size_px[axis] = 0;
            UI_Box *biggest_child = box->first;
            for (UI_Box *child = box->first->next; child != 0; child = child->next) {
                if (child->computed_size_px[axis] <= biggest_child->computed_size_px[axis]) continue;
                biggest_child = child;
            }
            box->computed_size_px[axis] = biggest_child->computed_size_px[axis];
            box->computed_size_px[axis] += biggest_child->style.margin.items[axis] * 2.f;
            box->computed_size_px[axis] += box->style.pad.items[axis] * 2.f;
        } break;
    }

    for_ui_axis (axis) assert(box->computed_size_px[axis] != 0);
}

static void ui_compute_layout_relative_positions_and_rect(UI *ui, UI_Box *box) {
    if (box == 0) return;
    box->target_rect = (UI_Rect){
        .right = box->computed_size_px[ui_axis_x],
        .bottom = box->computed_size_px[ui_axis_y],
    };
    for_ui_axis (axis) ui_rect_shift_axis(&box->target_rect, axis, box->style.margin.items[axis]);

    if (box->parent == 0) goto recurse;
    ui_rect_shift_x(&box->target_rect, box->parent->target_rect.left);
    ui_rect_shift_y(&box->target_rect, box->parent->target_rect.top);

    if (box->prev == 0) goto recurse;
    UI_Axis layout_axis = box->parent->build.flags & ui_box_flag_child_axis;
    ui_rect_shift_axis(&box->target_rect, layout_axis, box->prev->pos_rel_parent[layout_axis]);
    ui_rect_shift_axis(&box->target_rect, layout_axis, box->prev->computed_size_px[layout_axis]);
    for_ui_axis (axis) box->pos_rel_parent[axis] = box->target_rect.items[axis] - box->parent->target_rect.items[axis];

    recurse:
    // TODO(felix): make lerp smoothing framerate-independent. Refer to https://www.youtube.com/watch?v=LSNQuFEDOyQ (r = remainder after one second)
    box->rect = v4_lerp(box->rect, box->target_rect, ui_animation_speed);
    // NOTE(felix): this is needed to avoid odd sub-pixel motion while animating
    box->rect = v4_round(box->rect);
    for (UI_Box *child = box->first; child != 0; child = child->next) ui_compute_layout_relative_positions_and_rect(ui, child);
}

static void ui_compute_layout_solve_violations(UI *ui, UI_Box *box) {
    discard(ui);
    discard(box);
    // TODO(felix)
    // NOTE(felix): preorder
}

static void ui_compute_layout_standalone(UI *ui, UI_Box *box) {
    if (box == 0) return;
    for_ui_axis (axis) switch (box->build.size[axis].kind) {
        case ui_size_kind_nil: assert(false); break;
        case ui_size_kind_text: /* computed_size_px is already known */ break;
        case ui_size_kind_pixels: {
            // TODO(felix): don't multiply
            box->computed_size_px[axis] = box->build.size[axis].value * 5.f;
        } break;
    }
    for (UI_Box *child = box->first; child != 0; child = child->next) ui_compute_layout_standalone(ui, child);
    return;
}

static UI ui_init(Arena *persistent_arena, Arena *frame_arena, Gfx_Render_Context *gfx) {
    UI ui = {
        .persistent_arena = persistent_arena,
        .frame_arena = frame_arena,
        .gfx = gfx,
        .box_hashmap = ui_box_hashmap, // TODO(felix): this should be a local-persist, not a global
    };
    ui.style_stack.pad.arena = ui.persistent_arena;
    ui.style_stack.margin.arena = ui.persistent_arena;
    ui.style_stack.clr_fg.arena = ui.persistent_arena;
    ui.style_stack.clr_bg.arena = ui.persistent_arena;
    ui.style_stack.clr_border.arena = ui.persistent_arena;
    return ui;
}

static inline void ui_pop_parent(UI *ui) { ui->current_parent = ui->current_parent->parent; }

static UI_Interaction ui_push(UI *ui, UI_Box *box) {
    box->parent = 0;
    box->prev = 0;
    box->next = 0;
    box->first = 0;
    box->last = 0;

    if (ui->root == 0) {
        assert(ui->current_parent == 0);
        ui->root = box;
        ui->current_parent = box;
        goto interaction;
    }
    assert(ui->current_parent != 0);

    UI_Box *parent = ui->current_parent;
    UI_Box *child = box;
    child->parent = parent;
    if (parent->first == 0) {
        parent->first = child;
        parent->last = child;
    } else {
        parent->last->next = child;
        child->prev = parent->last;
        parent->last = child;
    }

    interaction: {
        if (!(box->build.flags & ui_box_flag_clickable)) {
            box->interaction = (UI_Interaction){0};
        } else {
            // TODO(felix): this should use the virtual mouse position
            V2 mouse_position = ui->gfx->frame_info.real_mouse_position;
            bool hovered =
                (box->rect.left <= mouse_position.x && mouse_position.x <= box->rect.right) &&
                (box->rect.top <= mouse_position.y && mouse_position.y <= box->rect.bottom);
            box->interaction = (UI_Interaction){
                .hovered = hovered,
                .clicked = hovered & ui->gfx->frame_info.mouse_left_clicked,
            };
        }
        return box->interaction;
    }
}

static UI_Interaction ui_push_parent(UI *ui, UI_Box *box) {
    UI_Interaction interaction = ui_push(ui, box);
    box->parent = ui->current_parent;
    ui->current_parent = box;
    return interaction;
}

static inline void ui_rect_shift_axis(UI_Rect *rect, UI_Axis axis, f32 shift) {
    rect->items[axis] += shift;
    rect->items[2 + axis] += shift;
}

static inline void ui_rect_shift_x(UI_Rect *rect, f32 shift) { rect->left += shift; rect->right += shift; }

static inline void ui_rect_shift_y(UI_Rect *rect, f32 shift) { rect->top += shift; rect->bottom += shift; }

static void ui_render_recursive(UI *ui, UI_Box *box) {
    assert(!v4_equal(box->target_rect, (V4){0}));

    u8 draw_text = box->build.flags & ui_box_flag_draw_text;
    u8 draw_border = box->build.flags & ui_box_flag_draw_border;
    u8 draw_background = box->build.flags & ui_box_flag_draw_background;
    u8 draw_texture = box->build.flags & ui_box_flag_draw_texture;

    // TODO(felix): get all the colours and values below from box style
    UI_Box_Style *target = &box->target_style;

    // if (!draw_border) target->clr_border.a = 0;
    target->clr_border.a = draw_border ? 0.7f : 0;

    // TODO(felix): framerate-independent lerping
    box->style = (UI_Box_Style){
        .pad = v2_lerp(box->style.pad, target->pad, ui_animation_speed),
        .margin = v2_lerp(box->style.margin, target->margin, ui_animation_speed),
        .clr_fg = v4_lerp(box->style.clr_fg, target->clr_fg, ui_animation_speed),
        .clr_bg = v4_lerp(box->style.clr_bg, target->clr_bg, ui_animation_speed),
        .clr_border = v4_lerp(box->style.clr_border, target->clr_border, ui_animation_speed),
    };

    if (draw_background) {
        target->clr_bg = (V4){ .r = 0.6f, .g = 0.6f, .b = 0.6f, .a = 1.f };
        if (box->build.flags & ui_box_flag_clickable) {
            if (box->interaction.clicked) {
                box->style.clr_bg = (V4){ .r = 0, .g = 0, .b = 0, .a = 1.f };
                box->style.clr_border = (V4){ .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };
            } else if (box->interaction.hovered) {
                target->clr_bg = (V4){ .r = 0.4f, .g = 0.4f, .b = 0.4f, .a = 1.f };
                target->clr_border.a = 1.f;
                // looks better when instant; the *return* to unhovered colours can animate instead
                box->style.clr_bg = target->clr_bg;
                box->style.clr_border = target->clr_border;
            } else target->clr_bg.a = 1.f;
        }

        // V4 background = box->style.clr_bg;

        // TODO(felix): move these to box style
        // TODO(felix): add corner radius
        i32 border_thickness = 1;
        // TODO(felix): draw border
        discard(border_thickness);

        u32 rgba = gfx_rgba8_from_v4(box->style.clr_bg);
        // u32 rgba = 0xe0e0e0ff;
        i32 top = (i32)(box->rect.top + 0.5f);
        i32 left = (i32)(box->rect.left + 0.5f);
        i32 width = (i32)(box->rect.right - box->rect.left + 0.5f);
        i32 height = (i32)(box->rect.bottom - box->rect.top + 0.5f);
        gfx_draw_rectangle(ui->gfx, top, left, width, height, rgba);
    }

    // // TODO(felix): remove
    // {
    //     V4 clr_bg = box->style.clr_bg;
    //     if (!draw_background) clr_bg.a = 0.f;

    //     D2D1_RECT_F d2_layout_rect = *(D2D1_RECT_F *)(&box->rect);
    //     D2D1_ROUNDED_RECT d2_rounded_rect = {
    //         .rect = d2_layout_rect,
    //         .radiusX = corner_radius, .radiusY = corner_radius,
    //     };

    //     // TODO(felix): cache this!
    //     ID2D1SolidColorBrush *bg_brush = 0;
    //     win32_ensure_hr(ID2D1RenderTarget_CreateSolidColorBrush(ui->gfx->d2d_render_target, (D2D1_COLOR_F *)&clr_bg, 0, &bg_brush));
    //     ID2D1RenderTarget_FillRoundedRectangle(ui->gfx->d2d_render_target, &d2_rounded_rect, (ID2D1Brush *)bg_brush);

    //     // TODO(felix): cache this!
    //     ID2D1SolidColorBrush *border_brush = 0;
    //     win32_ensure_hr(ID2D1RenderTarget_CreateSolidColorBrush(ui->gfx->d2d_render_target, (D2D_COLOR_F *)&box->style.clr_border, 0, &border_brush));

    //     ID2D1RenderTarget_DrawRoundedRectangle(ui->gfx->d2d_render_target,
    //         &d2_rounded_rect, (ID2D1Brush *)border_brush, border_thickness, ui->gfx->d2d_stroke_style
    //     );

    //     ensure_released_and_null(&border_brush);
    //     ensure_released_and_null(&bg_brush);
    // }

    if (draw_text) {
        UI_Rect text_rect = box->rect;
        text_rect.left += box->style.margin.x;

        i32 x = (i32)text_rect.left;
        i32 y = (i32)text_rect.top;
        gfx_draw_text(ui->gfx, ui->gfx->font, box->build.display_string, x, y, 0x000000ff);
    }

    if (draw_texture) panic("TODO(felix)");

    for (UI_Box *child = box->first; child != 0; child = child->next) ui_render_recursive(ui, child);
}

static UI_Box *ui_row(UI *ui) {
    UI_Box *row = ui_box_frame_local_not_keyed(ui);
    row->build.flags = ui_axis_x;
    row->build.size[ui_axis_x].kind = ui_size_kind_sum_of_children;
    row->build.size[ui_axis_y].kind = ui_size_kind_largest_child;
    return row;
}

static UI_Interaction ui_text(UI *ui, char *format, ...) {
    va_list args;
    va_start(args, format);
    String string = string_vprint(ui->frame_arena, format, args);
    va_end(args);

    UI_Box *text = ui_box(ui, string);
    text->build.flags = ui_box_flag_draw_text;
    for_ui_axis (axis) text->build.size[axis].kind = ui_size_kind_text;
    return ui_push(ui, text);
}
