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
structdef(Map_Slot) { Map_Index item_index, next_slot_index; };

// TODO(felix): configurable hash function
// TODO(felix): ordering flag
// TODO(felix): growable
// TODO(felix): configurable comparison function (string_equal should remain default)
structdef(Map) {
    Array_String key_from_item_index;
    Array_void items;
    usize item_size_bytes;
    Array_Map_Slot slots;
};

#define map_create(arena, capacity, type) map_create_explicit_item_size(arena, capacity, sizeof(type))
static Map map_create_explicit_item_size(Arena *arena, usize capacity, usize item_size_bytes) {
    Array_void items = { .arena = arena };
    bool non_zero = false;
    array_ensure_capacity_explicit_item_size(&items, capacity, item_size_bytes, non_zero);
    // item_index = 0 is a nil item_index
    items.count = 1;

    Array_String key_from_item_index = { .arena = arena };
    array_ensure_capacity(&key_from_item_index, capacity);
    key_from_item_index.count = capacity;

    // should this be configurable?
    usize overcapacity_factor = 2;
    Array_Map_Slot slots = { .arena = arena };
    array_ensure_capacity(&slots, capacity * overcapacity_factor);
    slots.count = capacity;

    Map map = {
        .key_from_item_index = key_from_item_index,
        .items = items,
        .item_size_bytes = item_size_bytes,
        .slots = slots,
    };
    return map;
}

static void *map_get_or_put(Map *map, String key, void *new_item) {
    usize hash = hash_function_djb2(key, map->items.capacity);
    Map_Slot *slot = &map->slots.data[hash];

    for (;;) {
        bool empty = slot->item_index == 0;
        if (empty) {
            *slot = (Map_Slot){ .item_index = (Map_Index)map->items.count };
            // assert(slot->next_slot_index == 0);
            // slot->item_index = (Map_Index)map->items.count;
            array_push_explicit_item_size_assume_capacity(&map->items, new_item, map->item_size_bytes);
            map->key_from_item_index.data[slot->item_index] = key;
            break;
        }

        String key_at_index = map->key_from_item_index.data[slot->item_index];
        assert(key.count != 0);
        assert(key_at_index.count != 0);
        if (string_equal(key, key_at_index)) break;

        bool create_new_slot = slot->next_slot_index == 0;
        if (create_new_slot) {
            Map_Index new_slot_index = (Map_Index)map->slots.count;

            Map_Slot new_slot = {0};
            array_push(&map->slots, &new_slot);

            slot->next_slot_index = new_slot_index;
        }

        slot = &map->slots.data[slot->next_slot_index];
    }

    assert(slot->item_index < map->items.count);
    void *item = (u8 *)map->items.data + (slot->item_index * map->item_size_bytes);
    return item;
}
