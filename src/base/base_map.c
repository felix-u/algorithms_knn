static usize hash_function_djb2(String bytes, usize capacity) {
    usize value = 5381;
    for_slice (u8 *, byte, bytes) {
        value = ((value << 5) + value) + *byte;
    }
    // avoid outputting 0
    value %= (capacity - 1);
    value += 1;
    return value;
}

typedef u32 Map_Index;
typedef Array(Map_Index) Array_Map_Index;

// TODO(felix): configurable hash function
// TODO(felix): ordering flag
// TODO(felix): growable
// TODO(felix): configurable comparison function (string_equal should remain default)
structdef(Map) {
    Array_String keys;
    Array_void items;
    Array_Map_Index item_indices_from_hash;
    usize item_size_bytes;
};

#define map_create(arena, capacity, type) map_create_explicit_item_size(arena, capacity, sizeof(type))
static Map map_create_explicit_item_size(Arena *arena, usize capacity, usize item_size_bytes) {
    Array_void items = { .arena = arena };
    bool non_zero = false;
    array_ensure_capacity_explicit_item_size(&items, capacity, item_size_bytes, non_zero);
    // item_index = 0 is a nil item_index
    items.count = 1;

    Array_String keys = { .arena = arena };
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
    };
    return map;
}

structdef(Map_Get_Arguments) {
    Map *map;
    String key;
    bool or_new;
};

structdef(Map_Get) { Map_Index index; void *item; };

#define map_get(...) map_get_argument_struct((Map_Get_Arguments){ __VA_ARGS__ })
static Map_Get map_get_argument_struct(Map_Get_Arguments arguments) {
    Map *map = arguments.map;
    String key = arguments.key;

    assert(key.count != 0);

    usize hash_index = hash_function_djb2(key, map->item_indices_from_hash.capacity);
    Map_Index *item_index = 0;
    Map_Index probe_increment = 1;

    for (;;) {
        item_index = &map->item_indices_from_hash.data[hash_index];
        String key_at_index = map->keys.data[*item_index];

        bool empty = key_at_index.count == 0 || *item_index == 0;
        if (empty) {
            if (!arguments.or_new) return (Map_Get){ .item = map->items.data };

            usize max_load = 70;
            assert(map->items.count * 100 <= map->item_indices_from_hash.capacity * max_load);

            *item_index = (Map_Index)map->items.count;
            map->items.count += 1;
            array_push_assume_capacity(&map->keys, &key);

            break;
        }

        if (string_equal(key_at_index, key)) break;

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

static Slice_String map_get_keys(Map *map) {
    if (map->keys.count == 0) return bit_cast(Slice_String) map->keys;
    Slice_String result = slice_range(map->keys, 1, map->keys.count);
    return result;
}
