#define BASE_GRAPHICS 0
#include "base/base.c"

#include "stopwords.c"

// TODO(felix): byte-pair encoder
// TODO(felix): BPE vectoriser and vocab file
// TODO(felix): swap out vectoriser but keep same knn. re-tune on validation, and re-run
// TODO(felix): compute accuracy, precision/recall/F_1, fill in McNemar table

// djb2 hash
static inline usize compute_hash(String bytes, usize capacity) {
    usize value = 5381;
    for_slice (u8 *, byte, bytes) {
        value = ((value << 5) + value) + *byte;
    }
    value %= capacity;
    return value;
}

typedef u16 Byte_Pair_Index;
typedef Array(u16) Array_u16;
typedef Array_u16 Array_Byte_Pair_Index;
structdef(Byte_Pair) {
    Byte_Pair_Index left, right;
    // u16 count;
};

static usize usize_from_byte_pair(Byte_Pair pair) {
    usize result = 0;
    result |= (usize)(pair.left << 16);
    result |= pair.right;
    return result;
}

static Byte_Pair byte_pair_from_usize(usize number) {
    Byte_Pair result = {0};
    result.left = (Byte_Pair_Index)(number >> 16);
    result.right = (Byte_Pair_Index)(number & 0xffff);
    return result;
}

static void byte_pair_print(Map pairs, Byte_Pair_Index index, bool with_separator) {
    Byte_Pair pair = byte_pair_from_usize(pairs.keys.data[index].number);

    if (pair.right == 0) {
        assert(pair.left < 128);
        print("%", fmt(char, (u8)pair.left));
        return;
    }
    if (with_separator) print("(");
    byte_pair_print(pairs, pair.left, with_separator);
    if (with_separator) print(")(");
    byte_pair_print(pairs, pair.right, with_separator);
    if (with_separator) print(")");
}

structdef(Vector) { Array_u16 items; f64 length; };

enumdef(Mode, u8) { mode_word, mode_token, mode_count };

structdef(Document) {
    String label, text;

    Slice_String words;
    Array_Byte_Pair_Index byte_pair_indices, next_byte_pair_indices;

    Vector vectors[mode_count];
};

typedef Array(f64) Array_f64;
static inline Array_f64 compute_similarities(Arena *arena, Document query, Array_Document comparison_set, Mode mode) {
    Array_f64 similarities = { .arena = arena };
    array_ensure_capacity(&similarities, comparison_set.count);
    for_slice (Document *, sample, comparison_set) {
        f64 dot_product = 0;
        for (usize i = 0; i < sample->vectors[mode].items.count; i += 1) {
            f64 query_feature = (f64)query.vectors[mode].items.data[i];
            f64 sample_feature = (f64)sample->vectors[mode].items.data[i];
            dot_product += query_feature * sample_feature;
        }

        f64 cosine_similarity = dot_product / (query.vectors[mode].length * sample->vectors[mode].length);
        array_push_assume_capacity(&similarities, &cosine_similarity);
    }
    return similarities;
}

static inline String compute_guess(Arena *arena, usize k, Array_f64 similarities, Array_Document comparison_set) {
    Array_Document training_set = comparison_set;

    Array_Document k_nearest = { .arena = arena };
    array_ensure_capacity(&k_nearest, k);

    while (k_nearest.count < k) {
        usize max_i = 0;
        for (usize i = 0; i < similarities.count; i += 1) {
            if (similarities.data[i] <= similarities.data[max_i]) continue;
            max_i = i;
        }
        Document *most_similar = &training_set.data[max_i];
        array_push_assume_capacity(&k_nearest, most_similar);
        similarities.data[max_i] = 0;
    }

    structdef(Guess) { String label; usize count; };
    Array_Guess guesses = { .arena = arena };
    array_ensure_capacity(&guesses, k);

    for_slice (Document *, neighbour, k_nearest) {
        Guess *existing_guess = 0;
        for_slice (Guess *, previous_guess, guesses) {
            if (!string_equal(previous_guess->label, neighbour->label)) continue;
            existing_guess = previous_guess;
            break;
        }

        if (existing_guess != 0) {
            existing_guess->count += 1;
        } else {
            Guess new_guess = { .label = neighbour->label, .count = 1 };
            array_push_assume_capacity(&guesses, &new_guess);
        }
    }

    #if BUILD_DEBUG
        usize count = 0;
        for_slice (Guess *, guess, guesses) count += guess->count;
        assert(count == k);
    #endif

    Guess winning_guess = {0};
    for_slice (Guess *, guess, guesses) {
        if (guess->count > winning_guess.count) winning_guess = *guess;
    }
    return winning_guess.label;
}

void entrypoint(void) {
    Arena arena = arena_init(512 * 1024 * 1024);

    Slice_String arguments = os_get_arguments(&arena);
    if (arguments.count != 2) {
        log_error("usage: % path/to/corpus_directory/", fmt(String, arguments.data[0]));
        exit(1);
    }

    String corpus_directory = arguments.data[1];

    String path_to_corpus = string_print(&arena, "%/%", fmt(String, corpus_directory), fmt(String, string("raw.txt")));
    String corpus = file_read_bytes_relative_path(&arena, cstring_from_string(&arena, path_to_corpus), UINT32_MAX);
    if (corpus.count == 0) exit(1);
    print("[info] Read %kB from '%'\n", fmt(usize, corpus.count / 1024), fmt(String, path_to_corpus));

    Map stopword_map = map_create(&arena, stopword_list.count, bool, map_key_string);
    for_slice (String *, stopword, stopword_list) {
        Map_Index index = map_get(&stopword_map, string, *stopword, .or_new = true).index;
        assert(index != 0);
    }

    Array_Document documents = { .arena = &arena };
    usize character_removal_count = 0;
    usize stopword_removal_count = 0;
    for (usize i = 0; i < corpus.count; i += 1) {
        Document document = {0};

        usize start_index = i;
        while (i < corpus.count && corpus.data[i] != ':') i += 1;
        document.label = string_range(corpus, start_index, i);
        i += 1;

        start_index = i;
        while (i < corpus.count && corpus.data[i] != '\n') i += 1;
        String abstract = string_range(corpus, start_index, i);

        Array_u8 abstract_ascii_lowercase = { .arena = &arena };
        array_ensure_capacity(&abstract_ascii_lowercase, abstract.count);
        for_slice (u8 *, c, abstract) {
            if (!ascii_is_alpha(*c) && !ascii_is_decimal(*c) && *c != ' ' && *c != '\n') {
                character_removal_count += 1;
                continue;
            }
            if (ascii_is_alpha(*c)) *c |= 0x20;
            array_push_assume_capacity(&abstract_ascii_lowercase, c);
        }

        Array_String words_no_stopwords = { .arena = &arena };
        Array_u8 text_no_stopwords = { .arena = &arena };
        array_ensure_capacity(&text_no_stopwords, abstract_ascii_lowercase.count);
        String text = bit_cast(String) abstract_ascii_lowercase;
        for (usize j = 0; j < text.count;) {
            usize start_index_including_whitespace = j;
            while (j < text.count && ascii_is_whitespace(text.data[j])) j += 1;

            bool trailing_whitespace = j == text.count;
            if (trailing_whitespace) break;

            start_index = j;
            while (j < text.count && !ascii_is_whitespace(text.data[j])) j += 1;
            String word = string_range(text, start_index, j);

            bool is_stopword = map_get(&stopword_map, string, word).index != 0;
            if (is_stopword) {
                stopword_removal_count += 1;
                continue;
            }

            array_push(&words_no_stopwords, &word);

            String to_push = string_range(text, start_index_including_whitespace, j);
            array_push_slice_assume_capacity(&text_no_stopwords, &to_push);
        }
        document.text = bit_cast(String) text_no_stopwords;
        document.words = bit_cast(Slice_String) words_no_stopwords;

        array_push(&documents, &document);
    }
    usize total_word_count = 0;
    for_slice (Document *, document, documents) total_word_count += document->words.count;
    print("[info] Removed % non-ASCII characters and % stopwords; total word count %\n", fmt(usize, character_removal_count), fmt(usize, stopword_removal_count), fmt(usize, total_word_count));

    Map word_map = map_create(&arena, total_word_count, u16, map_key_string);

    usize unique_count = 0;
    usize repeat_count = 0;
    for_slice (Document *, document, documents) for_slice (String *, word, document->words) {
        Map_Get frequency = map_get(&word_map, string, *word, .or_new = true);
        assert(frequency.index != 0);
        u16 *count = frequency.item;
        usize new_count = clamp_high((usize)*count + 1, 0xffff);
        *count = (u16)new_count;

        if (*count == 1) unique_count += 1;
        else repeat_count += 1;
    }
    print("[info] Found % unique words, % repeat words\n", fmt(usize, unique_count), fmt(usize, repeat_count));

    usize vocabulary_size = 10000;

    Map word_vocabulary = map_create(&arena, vocabulary_size, u16, map_key_string);

    {
        Slice_Map_Key words = map_get_keys(&word_map);
        typedef Slice(u16) Slice_u16;
        Slice_u16 counts = {0};
        map_get_items(&word_map, &counts);
        assert(words.count == counts.count);

        while (word_vocabulary.items.count < vocabulary_size) {
            String best_word = {0};
            u16 *best_count = 0;

            for (usize i = 0; i < words.count; i += 1) {
                String word = words.data[i].string;
                u16 *count = &counts.data[i];

                if (best_word.count != 0 && (*count <= *best_count)) continue;
                best_word = word;
                best_count = count;
            }
            if (best_word.count == 0) break;

            Map_Get new = map_get(&word_vocabulary, string, best_word, .or_new = true);
            assert(new.index != 0);
            assert(*(u16 *)new.item == 0);
            *(u16 *)new.item = *best_count;
            *best_count = 0;
        }
        print("[info] Computed vocabulary of top % most-used words\n", fmt(usize, vocabulary_size));

        for_slice (Document *, document, documents) {
            Mode mode = mode_word;
            Vector *vector = &document->vectors[mode];

            vector->items.arena = &arena;
            array_ensure_capacity(&vector->items, vocabulary_size + 1);
            vector->items.count = vocabulary_size + 1;

            for_slice (String *, word, document->words) {
                Map_Get get = map_get(&word_vocabulary, string, *word);
                bool unknown = get.index == 0;
                if (unknown) vector->items.data[vocabulary_size] += 1;
                else vector->items.data[get.index] += 1;
            }
        }
        print("[info] Computed word vectors for all % documents\n", fmt(usize, documents.count));

        for_slice (Document *, document, documents) {
            Mode mode = mode_word;
            Vector *vector = &document->vectors[mode];
            f64 sum_of_squares = 0;
            for_slice (u16 *, frequency, vector->items) {
                f64 value = (f64)*frequency;
                sum_of_squares += value * value;
            }
            vector->length = sqrt(sum_of_squares);
        }
        print("[info] Computed word vector lengths for all % documents\n", fmt(usize, documents.count));
    }

    bool byte_pair_enabled = true;
    if (byte_pair_enabled) {
        usize total_character_count = 0;
        for_slice (Document *, document, documents) total_character_count += document->text.count;
        print("[info] Corpus has % characters total\n", fmt(usize, total_character_count));

        Map byte_pair_dictionary = map_create(&arena, total_character_count, u16, map_key_usize);

        for (u16 i = 1; i < 128; i += 1) {
            Byte_Pair ascii = { .left = i };
            usize value = usize_from_byte_pair(ascii);
            map_get(&byte_pair_dictionary, number, value, .or_new = true);
        }

        for_slice (Document *, document, documents) {
            document->byte_pair_indices.arena = &arena;
            String text = document->text;
            array_ensure_capacity(&document->byte_pair_indices, text.count);

            for (usize i = 0; i < text.count; i += 1) {
                Byte_Pair_Index ascii = (Byte_Pair_Index)document->text.data[i];
                array_push_assume_capacity(&document->byte_pair_indices, &ascii);
            }

            document->next_byte_pair_indices.arena = &arena;
            array_ensure_capacity(&document->next_byte_pair_indices, text.count);
        }

        Array_u16 byte_pair_vocabulary_counts = { .arena = &arena };
        array_ensure_capacity(&byte_pair_vocabulary_counts, vocabulary_size);
        Array_Byte_Pair_Index byte_pair_vocabulary = { .arena = &arena };
        array_ensure_capacity(&byte_pair_vocabulary, vocabulary_size);

        while (byte_pair_vocabulary.count < vocabulary_size) {
            Arena_Temp temp = arena_temp_begin(&arena);

            typedef Slice(u16) Slice_u16;
            Slice_u16 frequencies = {0};
            map_get_items(&byte_pair_dictionary, &frequencies);
            for_slice (u16 *, frequency, frequencies) *frequency = 0;

            for_slice (Document *, document, documents) {
                for (usize i = 0; i + 1 < document->byte_pair_indices.count; i += 1) {
                    Byte_Pair pair = { .left = document->byte_pair_indices.data[i], .right = document->byte_pair_indices.data[i + 1] };
                    usize value = usize_from_byte_pair(pair);

                    Map_Get match = map_get(&byte_pair_dictionary, number, value);

                    if (match.index == 0) {
                        match = map_get(&byte_pair_dictionary, number, value, .or_new = true);
                        u16 *count = match.item;
                        assert(*count == 0);
                    }

                    u16 *count = match.item;
                    usize new_count = clamp_high((usize)*count + 1, 0xffff);
                    *count = (u16)new_count;
                }
            }

            Byte_Pair *most_common = &(Byte_Pair){0};
            u16 most_common_count = 0;
            Byte_Pair_Index most_common_index = 0;
            Slice_String keys = bit_cast(Slice_String) byte_pair_dictionary.keys;
            assert(keys.count == byte_pair_dictionary.items.count);
            Slice_u16 counts = bit_cast(Slice_u16) byte_pair_dictionary.items;
            for (usize i = 0; i < keys.count; i += 1) {
                Byte_Pair pair = byte_pair_from_usize(byte_pair_dictionary.keys.data[i].number);
                u16 count = counts.data[i];
                if (most_common != 0 && (count <= most_common_count)) continue;
                *most_common = pair;
                most_common_index = (Byte_Pair_Index)i;
                most_common_count = count;
            }
            if (most_common == 0 || most_common_count < 2) {
                arena_temp_end(temp);
                break;
            }
            array_push_assume_capacity(&byte_pair_vocabulary_counts, &most_common_count);
            array_push_assume_capacity(&byte_pair_vocabulary, &most_common_index);

            for_slice (Document *, document, documents) {
                Array_Byte_Pair_Index *updated_pairs = &document->next_byte_pair_indices;
                updated_pairs->count = 0;

                for (usize i = 0; i + 1 < document->byte_pair_indices.count; i += 1) {
                    Byte_Pair pair = { .left = document->byte_pair_indices.data[i], .right = document->byte_pair_indices.data[i + 1] };
                    bool should_compress = pair.left == most_common->left && pair.right == most_common->right;
                    if (should_compress) {
                        array_push_assume_capacity(updated_pairs, &most_common_index);
                        i += 1;
                    } else {
                        array_push_assume_capacity(updated_pairs, &pair.left);
                        if (i + 1 == document->byte_pair_indices.count - 1) array_push_assume_capacity(updated_pairs, &pair.right);
                    }
                }

                swap(Array_Byte_Pair_Index, &document->byte_pair_indices, updated_pairs);
            }

            arena_temp_end(temp);

            usize total_byte_pair_count = 0;
            for_slice (Document *, document, documents) total_byte_pair_count += document->byte_pair_indices.count;
            print("\r[info] Byte pair merge %, total %", fmt(usize, byte_pair_vocabulary.count), fmt(usize, total_byte_pair_count));
        }
        print("\n");

        usize effective_byte_pair_count = 0;
        for (; effective_byte_pair_count < byte_pair_vocabulary.count; effective_byte_pair_count += 1) {
            if (byte_pair_vocabulary_counts.data[effective_byte_pair_count] == 0) break;
        }
        byte_pair_vocabulary.count = effective_byte_pair_count;
        byte_pair_vocabulary_counts.count = effective_byte_pair_count;
        print("[info] Computed vocabulary of top % most-used byte pairs", fmt(usize, byte_pair_vocabulary.count));
        if (byte_pair_vocabulary.count < vocabulary_size) print(" (maximum compression reached before %-token max vocabulary size)", fmt(usize, vocabulary_size));
        print("\n");

        bool print_byte_pair_expansions = false;
        if (print_byte_pair_expansions) {
            usize top = byte_pair_vocabulary.count;
            print("[info] Expansion of top % pairs:\n", fmt(usize, top));
            for (usize i = 0; i < top; i += 1) {
                print("[%] % usages of '", fmt(usize, i), fmt(u16, byte_pair_vocabulary_counts.data[i]));
                byte_pair_print(byte_pair_dictionary, byte_pair_vocabulary.data[i], false);
                print("' = ");
                byte_pair_print(byte_pair_dictionary, byte_pair_vocabulary.data[i], true);
                print("\n");
            }
        }

        for_slice (Document *, document, documents) {
            Mode mode = mode_token;
            Vector *vector = &document->vectors[mode];

            vector->items.arena = &arena;
            array_ensure_capacity(&vector->items, byte_pair_vocabulary.count + 1);
            vector->items.count = byte_pair_vocabulary.count + 1;

            for_slice (Byte_Pair_Index *, byte_pair_index, document->byte_pair_indices) {
                bool unknown = true;
                for (usize i = 0; i < byte_pair_vocabulary.count; i += 1) {
                    Byte_Pair_Index vocabulary_byte_pair_index = byte_pair_vocabulary.data[i];
                    if (*byte_pair_index != vocabulary_byte_pair_index) continue;
                    vector->items.data[i] += 1;
                    unknown = false;
                    break;
                }
                if (unknown) vector->items.data[byte_pair_vocabulary.count] += 1;
            }
        }
        print("[info] Computed token vectors for all % documents\n", fmt(usize, documents.count));

        for_slice (Document *, document, documents) {
            Mode mode = mode_token;
            Vector *vector = &document->vectors[mode];

            f64 sum_of_squares = 0;
            for_slice (u16 *, frequency, vector->items) {
                f64 value = (f64)*frequency;
                sum_of_squares += value * value;
            }
            vector->length = sqrt(sum_of_squares);
        }
        print("[info] Computed token vector lengths for all % documents\n", fmt(usize, documents.count));
    }

    for (usize i = 0; i < documents.count; i += 1) {
        String bytes = documents.data[i].text;
        usize random_index_from_hash = compute_hash(bytes, documents.count);
        swap(Document, &documents.data[i], &documents.data[random_index_from_hash]);
    }
    print("[info] Shuffled documents using hashes as source of randomness\n");

    enum { train_id, validation_id, testing_id, split_count };
    typedef Slice(f32) Slice_f32;
    Slice_f32 fractions = slice_of(f32, [train_id] = 0.6f, [validation_id] = 0.2f, [testing_id] = 0.2f);

    Array_Document splits[split_count] = {0};
    usize document_index = 0;
    for (usize split = 0; split < split_count; split += 1) {
        splits[split].arena = &arena;
        usize capacity = (usize)((f32)documents.count * fractions.data[split]);
        array_ensure_capacity(&splits[split], capacity);

        Slice_Document to_push = slice_range(documents, document_index, document_index + capacity);
        array_push_slice_assume_capacity(&splits[split], &to_push);

        document_index += to_push.count;
    }
    print("[info] Split dataset into training (%), validation (%), and testing (%)\n",
        fmt(usize, splits[train_id].count), fmt(usize, splits[validation_id].count), fmt(usize, splits[testing_id].count)
    );

    Array_Document training_set = splits[train_id];
    Array_Document validation_set = splits[validation_id];
    Array_Document testing_set = splits[testing_id];

    typedef Slice(usize) Slice_usize;
    Slice_usize k_values = slice_of(usize, 1, 3, 5, 7, 9);

    for (Mode mode = 0; mode < 1 + byte_pair_enabled; mode += 1) {
        Arena_Temp temp = arena_temp_begin(&arena);

        String mode_string = mode == mode_word ? string("<word>") : string("<token>");

        typedef Array(Array_String) Array_Array_String;
        Array_Array_String validation_guesses_per_k = { .arena = &arena };
        array_ensure_capacity(&validation_guesses_per_k, k_values.count);
        validation_guesses_per_k.count = k_values.count;

        for_slice(Array_String *, validation_guesses, validation_guesses_per_k) {
            validation_guesses->arena = &arena;
            array_ensure_capacity(validation_guesses, validation_set.count);
        }

        for_slice (Document *, query, validation_set) {
            Array_f64 similarities = compute_similarities(&arena, *query, training_set, mode);
            for (usize k_index = 0; k_index < k_values.count; k_index += 1) {
                usize k = k_values.data[k_index];
                String guess = compute_guess(&arena, k, similarities, training_set);
                array_push_assume_capacity(&validation_guesses_per_k.data[k_index], &guess);
            }
        }

        usize best_k_index = 0;
        usize max_correct_guess_count = 0;

        print("[info] % (validation) ", fmt(String, mode_string));
        for (usize k_index = 0; k_index < k_values.count; k_index += 1) {
            Array_String validation_guesses = validation_guesses_per_k.data[k_index];
            usize k = k_values.data[k_index];

            usize correct_guess_count = 0;
            assert(validation_guesses.count == validation_set.count);
            for (usize i = 0; i < validation_guesses.count; i += 1) {
                String validation_label = validation_set.data[i].label;
                String guess_label = validation_guesses.data[i];
                correct_guess_count += string_equal(validation_label, guess_label);
            }

            if (correct_guess_count > max_correct_guess_count) {
                best_k_index = k_index;
                max_correct_guess_count = correct_guess_count;
            }

            f32 percentage = (f32)correct_guess_count / (f32)validation_guesses.count * 100.f;
            print("(k = %, %%) ", fmt(usize, k), fmt(f32, percentage), fmt(char, '%'));
        }
        print("\n");

        usize best_k = k_values.data[best_k_index];
        print("[info] % Choosing k = %\n", fmt(String, mode_string), fmt(usize, best_k));

        {
            Array_String testing_guesses = { .arena = &arena };
            array_ensure_capacity(&testing_guesses, testing_set.count);
            for_slice (Document *, query, testing_set) {
                Array_f64 similarities = compute_similarities(&arena, *query, training_set, mode);
                usize k = best_k;
                String guess = compute_guess(&arena, k, similarities, training_set);
                array_push_assume_capacity(&testing_guesses, &guess);
            }

            usize correct_guess_count = 0;
            for (usize i = 0; i < testing_guesses.count; i += 1) {
                String testing_label = testing_set.data[i].label;
                String guess_label = testing_guesses.data[i];
                correct_guess_count += string_equal(testing_label, guess_label);
            }
            f32 percentage = (f32)correct_guess_count / (f32)testing_guesses.count * 100.f;
            print("[info] % (testing) k = % had accuracy %%\n", fmt(String, mode_string), fmt(usize, best_k), fmt(f32, percentage), fmt(char, '%'));
        }

        arena_temp_end(temp);
    }

    arena_deinit(&arena);
    exit(0);
}
