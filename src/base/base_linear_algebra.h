// TODO(felix): square root, etc. via intrinsics, not math.h (avoid linking UCRT)

uniondef(V2) {
    struct { f32 x, y; };
    struct { f32 u, v; };
    struct { f32 left, right; };
    struct { f32 width, height; };

    f32 items[2];
};

uniondef(V3) {
    struct { f32 x, y, z; };
    struct { f32 u, v, w; };
    struct { f32 r, g, b; };

    struct { V2 xy; f32 _0; };
    struct { f32 _1; V2 yz; };
    struct { V2 uv; f32 _2; };
    struct { f32 _3; V2 vw; };
    struct { V2 rg; f32 _4; };
    struct { f32 _5; V2 gb; };

    f32 items[3];
};

uniondef(V4) {
    struct { f32 x, y, z, w; };
    struct { f32 r, g, b, a; };
    struct { f32 left, top, right, bottom; };

    struct { V2 xy, _0; };
    struct { f32 _1; V2 yz; f32 _2; };
    struct { V2 _3, zw; };
    struct { V2 rg, _4; };
    struct { f32 _5; V2 gb; f32 _6; };
    struct { V2 _7, ba; };

    struct { V3 xyz; f32 _8; };
    struct { V3 rgb; f32 _9; };
    struct { f32 _a; V3 yzw; };
    struct { f32 _b; V3 gba; };

    f32 items[4];
};

typedef V4 Quat;

uniondef(M3) {
    f32 items[3][3];
    V3 columns[3];
};

uniondef(M4) {
    f32 items[4][4];
    V4 columns[4];
};

#define vector_pi_f32 3.14159265358979f

static force_inline f32 radians_from_degrees(f32 degrees);

static force_inline f32 f32_lerp(f32 a, f32 b, f32 amount);

static force_inline V2   v2_add(V2 a, V2 b);
static force_inline V2   v2_div(V2 a, V2 b);
static       inline f32  v2_dot(V2 a, V2 b);
static force_inline bool v2_equal(V2 a, V2 b);
static       inline f32  v2_len(V2 v);
static       inline f32  v2_len_squared(V2 v);
static       inline V2   v2_lerp(V2 a, V2 b, f32 amount);
static force_inline V2   v2_max(V2 a, V2 b);
static force_inline V2   v2_min(V2 a, V2 b);
static force_inline V2   v2_mul(V2 a, V2 b);
static       inline V2   v2_norm(V2 v);
static force_inline V2   v2_reciprocal(V2 v);
static       inline V2   v2_rotate(V2 v, f32 angle_radians);
static       inline V2   v2_round(V2 v);
static       inline V2   v2_round_down(V2 v);
static force_inline V2   v2_scale(V2 v, f32 s);
static force_inline V2   v2_sub(V2 a, V2 b);

static force_inline V3   v3_add(V3 a, V3 b);
static       inline V3   v3_cross(V3 a, V3 b);
static       inline f32  v3_dot(V3 a, V3 b);
static force_inline bool v3_equal(V3 a, V3 b);
static force_inline V3   v3_forward_from_view(M4 view);
static       inline f32  v3_len(V3 v);
static       inline f32  v3_len_squared(V3 v);
static       inline V3   v3_lerp(V3 a, V3 b, f32 amount);
static force_inline V3   v3_neg(V3 v);
static       inline V3   v3_norm(V3 v);
static force_inline V3   v3_right_from_view(M4 view);
static force_inline V3   v3_scale(V3 v, f32 s);
static force_inline V3   v3_sub(V3 a, V3 b);
static       inline V3   v3_unproject(V3 pos, M4 view_projection);
static force_inline V3   v3_up_from_view(M4 view);

static force_inline V4   v4_add(V4 a, V4 b);
static       inline f32  v4_dot(V4 a, V4 b);
static force_inline bool v4_equal(V4 a, V4 b);
static       inline V4   v4_lerp(V4 a, V4 b, f32 amount);
static       inline V4   v4_round(V4 v);
static force_inline V4   v4_scale(V4 v, f32 s);
static force_inline V4   v4_sub(V4 a, V4 b);
static force_inline V4   v4v(V3 xyz, f32 w);

static inline Quat quat_from_rotation(V3 axis, f32 angle);
static inline Quat quat_mul_quat(Quat a, Quat b);
static inline V3   quat_rotate_v3(Quat q, V3 v);

static inline V3 m3_mul_v3(M3 m, V3 v);

static force_inline M4 m4_fill_diagonal(f32 value);
static       inline M4 m4_from_rotation(V3 axis, f32 angle);
static       inline M4 m4_from_translation(V3 translation);
static       inline M4 m4_inverse(M4 m);
static       inline M4 m4_look_at(V3 eye, V3 centre, V3 up_direction);
static       inline M4 m4_mul_m4(M4 a, M4 b);
static       inline V4 m4_mul_v4(M4 m, V4 v);
static       inline M4 m4_perspective_projection(f32 fov_vertical_radians, f32 width_over_height, f32 range_near, f32 range_far);
static force_inline M4 m4_transpose(M4 m);
