typedef u32 Map_Index;
typedef Array(Map_Index) Array_Map_Index;

uniondef(Map_Key) { String string; usize number; };
enumdef(Map_Key_Mode, u8) { map_key_string = 0, map_key_usize };

// TODO(felix): configurable hash function
// TODO(felix): ordering flag
// TODO(felix): growable
// TODO(felix): configurable comparison function (string_equal should remain default)
structdef(Map) {
    Array_Map_Key keys;
    Array_void items;
    Array_Map_Index item_indices_from_hash;
    usize item_size_bytes;
    Map_Key_Mode key_mode;
};

#define map_create(arena, capacity, type, key_mode) map_create_explicit_item_size(arena, capacity, sizeof(type), key_mode)
static Map map_create_explicit_item_size(Arena *arena, usize capacity, usize item_size_bytes, Map_Key_Mode key_mode) {
    Array_void items = { .arena = arena };
    bool non_zero = false;
    array_ensure_capacity_explicit_item_size(&items, capacity, item_size_bytes, non_zero);
    // item_index = 0 is a nil item_index
    items.count = 1;

    Array_Map_Key keys = { .arena = arena };
    array_ensure_capacity(&keys, capacity);
    keys.count = 1;

    // should this be configurable?
    usize overcapacity_factor = 2;
    Array_Map_Index item_indices_from_hash = { .arena = arena };
    array_ensure_capacity(&item_indices_from_hash, capacity * overcapacity_factor);
    item_indices_from_hash.count = capacity;

    Map map = {
        .keys = keys,
        .items = items,
        .item_size_bytes = item_size_bytes,
        .item_indices_from_hash = item_indices_from_hash,
        .key_mode = key_mode,
    };
    return map;
}

static usize hash_function_djb2(Map_Key key, Map_Key_Mode key_mode, usize capacity) {
    usize value = 5381;

    switch (key_mode) {
        case map_key_string: {
            String bytes = key.string;
            for_slice (u8 *, byte, bytes) {
                value = ((value << 5) + value) + *byte;
            }
        } break;
        case map_key_usize: {
            for (usize i = 0; i < 64; i += 8) {
                u8 byte = (u8)((key.number >> i) & 0xff);
                value = ((value << 5) + value) + byte;
            }
        } break;
        default: unreachable;
    }

    // avoid outputting 0
    value %= (capacity - 1);
    value += 1;
    return value;
}

structdef(Map_Get_Arguments) {
    Map *map;
    Map_Key key;
    bool or_new;
};

structdef(Map_Get) { Map_Index index; void *item; };

#define map_get(map, key_type, key, ...) map_get_argument_struct((Map_Get_Arguments){ map, (Map_Key){ .key_type = (key) }, __VA_ARGS__ })
static Map_Get map_get_argument_struct(Map_Get_Arguments arguments) {
    Map *map = arguments.map;
    Map_Key key = arguments.key;

    if (map->key_mode == map_key_string) assert(key.string.count != 0);

    usize hash_index = hash_function_djb2(key, map->key_mode, map->item_indices_from_hash.capacity);
    Map_Index *item_index = 0;
    Map_Index probe_increment = 1;

    for (;;) {
        item_index = &map->item_indices_from_hash.data[hash_index];
        Map_Key key_at_index = map->keys.data[*item_index];

        bool empty = *item_index == 0;
        if (map->key_mode == map_key_string) empty = empty || key_at_index.string.count == 0;

        if (empty) {
            if (!arguments.or_new) return (Map_Get){ .item = map->items.data };

            usize max_load = 70;
            assert(map->items.count * 100 <= map->item_indices_from_hash.capacity * max_load);

            *item_index = (Map_Index)map->items.count;
            map->items.count += 1;
            array_push_assume_capacity(&map->keys, &key);

            break;
        }

        bool equal = false;
        switch (map->key_mode) {
            case map_key_string: equal = string_equal(key_at_index.string, key.string); break;
            case map_key_usize: equal = key_at_index.number == key.number; break;
            default: unreachable;
        }
        if (equal) break;

        hash_index += probe_increment;
        probe_increment += 1;
        while (hash_index >= map->item_indices_from_hash.capacity) {
            hash_index -= (Map_Index)map->item_indices_from_hash.capacity;
        }
        if (hash_index == 0) hash_index = 1;
    }

    assert(*item_index < map->items.count);
    void *item = (u8 *)map->items.data + (*item_index * map->item_size_bytes);
    Map_Get result = { .index = *item_index, .item = item };
    return result;
}

#define map_get_items(map, items) map_get_items_explicit_item_size(map, (Slice_void *)(items), sizeof(*((items)->data)))
static void map_get_items_explicit_item_size(Map *map, Slice_void *items, usize item_size_bytes) {
    assert(map->item_size_bytes == item_size_bytes);
    if (map->items.count == 0) {
        *items = (Slice_void){0};
        return;
    }
    items->data = (u8 *)map->items.data + item_size_bytes;
    items->count = map->items.count - 1;
}

static Slice_Map_Key map_get_keys(Map *map) {
    if (map->keys.count == 0) return bit_cast(Slice_Map_Key) map->keys;
    Slice_Map_Key result = slice_range(map->keys, 1, map->keys.count);
    return result;
}
