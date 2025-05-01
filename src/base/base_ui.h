enumdef(UI_Axis, u8) { ui_axis_x, ui_axis_y, ui_axis_count };
#define for_ui_axis(axis_var_name) for (UI_Axis axis_var_name = 0; axis_var_name < ui_axis_count; axis_var_name += 1)

structdef(UI_Interaction) { bool hovered, clicked; };

enumdef(UI_Size_Kind, u8) {
    ui_size_kind_nil,
    ui_size_kind_text,
    ui_size_kind_sum_of_children,
    ui_size_kind_largest_child,
    ui_size_kind_pixels,
};

typedef V4 UI_Rect;

structdef(UI_Size) {
    UI_Size_Kind kind;
    f32 value, strictness;
};

// TODO(felix): add flags for text alignment
enumdef(UI_Box_Flags, u8) {
    ui_box_flag_child_axis      = 1 << 0,
    ui_box_flag_clickable       = 1 << 1,
    ui_box_flag_draw_text       = 1 << 2,
    ui_box_flag_draw_border     = 1 << 3,
    ui_box_flag_draw_background = 1 << 4,
    ui_box_flag_draw_texture    = 1 << 5,
};

structdef(UI_Box_Build) {
    String hash_string, display_string;
    UI_Box_Flags flags;
    UI_Size size[ui_axis_count];
};

structdef(UI_Box_Style) {
    V2 pad, margin;
    V4 clr_fg, clr_bg, clr_border;
};

structdef(UI_Box) {
    UI_Box_Build build;
    usize key;

    UI_Box_Style style, target_style;

    UI_Box *parent;
    UI_Box *prev, *next;
    UI_Box *first, *last;

    f32 computed_size_px[ui_axis_count];
    f32 pos_rel_parent[ui_axis_count];
    UI_Rect rect, target_rect;

    UI_Interaction interaction;
};
typedef Array(UI_Box *) Array_UI_Box_Pointer;

// TODO(felix): actually use properly
structdef(UI_Style_Stack) {
    Array_V2 pad, margin;
    Array_V4 clr_fg, clr_bg, clr_border;
};

structdef(UI) {
    Arena *persistent_arena, *frame_arena;
    Gfx_Render_Context *gfx;
    f32 pos[ui_axis_count];

    UI_Box *root;
    UI_Box *current_parent;
    Array_UI_Box_Pointer box_hashmap;

    UI_Style_Stack style_stack;
};

#define ui_animation_speed 0.4f
#define ui_box_map_size    1024

#define ui_expand_idx(line) i##line
#define ui_defer_loop_idx(line) ui_expand_idx(line)
#define ui_defer_loop(begin, end)\
    for (\
        int ui_defer_loop_idx(__LINE__) = ((begin), 0);\
        ui_defer_loop_idx(__LINE__) != 1;\
        ui_defer_loop_idx(__LINE__) += 1, (end)\
    )
#define ui_parent(ui, box) ui_defer_loop(ui_push_parent(ui, box), ui_pop_parent(ui))

#define ui_render(ui_pointer) ui_render_recursive((ui_pointer), (ui_pointer)->root)

static          UI_Box *ui_border_box(UI *ui);

structdef(UI_Box_Arguments) { UI *ui; String string; bool only_hash; };
#define ui_box(...) ui_box_argument_struct((UI_Box_Arguments){ __VA_ARGS__ })
static UI_Box *ui_box_argument_struct(UI_Box_Arguments arguments);

static          UI_Box *ui_box_frame_local_not_keyed(UI *ui);
static            void  ui_begin_build(UI *ui);
static            void  ui_end_build(UI *ui);
static            void  ui_compute_layout_dependent_ancestor(UI *ui, UI_Box *box);
static            void  ui_compute_layout_dependent_descendant(UI *ui, UI_Box *box);
static            void  ui_compute_layout_relative_positions_and_rect(UI *ui, UI_Box *box);
static            void  ui_compute_layout_solve_violations(UI *ui, UI_Box *box);
static            void  ui_compute_layout_standalone(UI *ui, UI_Box *box);
static              UI  ui_init(Arena *persistent_arena, Arena *frame_arena, Gfx_Render_Context *gfx);
static inline     void  ui_pop_parent(UI *ui);
static  UI_Interaction  ui_push(UI *ui, UI_Box *box);
static  UI_Interaction  ui_push_parent(UI *ui, UI_Box *box);
static inline     void  ui_rect_shift_axis(UI_Rect *rect, UI_Axis axis, f32 shift);
static inline     void  ui_rect_shift_x(UI_Rect *rect, f32 shift);
static inline     void  ui_rect_shift_y(UI_Rect *rect, f32 shift);
static            void  ui_render_recursive(UI *ui, UI_Box *box);

// Helpers for usage code
static  UI_Interaction  ui_button(UI *ui, char *format, ...);
static          UI_Box *ui_column(UI *ui);
static          UI_Box *ui_row(UI *ui);
static  UI_Interaction  ui_text(UI *ui, char *format, ...);
