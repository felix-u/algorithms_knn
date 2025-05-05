#define BASE_GRAPHICS 0
#include "base/base.c"

#include "stopwords.c"

// TODO(felix): data ingestion
    // TODO(felix): shuffle and split into 60% testing, 20% k-validation, 20% test
// TODO(felix): build vocabulary of 5000 words and write to file
// TODO(felix): build vectoriser and verify
// TODO(felix): knn
    // TODO(felix): compute similarity
    // TODO(felix): select top k indices
// TODO(felix): validate k, run bag-of-words on test corpus
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

void entrypoint(void) {
    Arena arena = arena_init(64 * 1024 * 1024);
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

    typedef Array(u16) Array_u16;
    structdef(Document) {
        String label, text;
        Slice_String words;
        Array_u16 word_vector;
        f64 word_vector_length;
    };

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
            start_index = j;
            while (j < text.count && !ascii_is_whitespace(text.data[j])) j += 1;
            String word = string_range(text, start_index, j);

            bool is_stopword = false;
            for_slice (String *, stopword, stopwords) {
                if (!string_equal(word, *stopword)) continue;
                stopword_removal_count += 1;
                is_stopword = true;
                break;
            }
            if (is_stopword) continue;
            array_push(&words_no_stopwords, &word);

            String to_push = string_range(text, start_index_including_whitespace, j);
            array_push_slice_assume_capacity(&text_no_stopwords, &to_push);
        }
        document.text = bit_cast(String) text_no_stopwords;
        document.words = bit_cast(Slice_String) words_no_stopwords;

        array_push(&documents, &document);
    }
    print("[info] Removed % non-ASCII characters and % stopwords\n", fmt(usize, character_removal_count), fmt(usize, stopword_removal_count));

    structdef(Word_Frequency) { String *word; u16 count; };

    structdef(Hashmap_Pair) {
        Word_Frequency frequency;
        Hashmap_Pair *next;
    };

    typedef Array(Hashmap_Pair *) Array_Hashmap_Pair_Pointer;

    Array_Hashmap_Pair_Pointer word_map = { .arena = &arena };
    array_ensure_capacity(&word_map, 10000);

    usize unique_count = 0;
    usize repeat_count = 0;
    for_slice (Document *, document, documents) for_slice (String *, word, document->words) {
        usize hash = compute_hash(*word, word_map.capacity);
        Hashmap_Pair **pair = &word_map.data[hash];
        while (*pair != 0 && !string_equal(*(*pair)->frequency.word, *word)) pair = &(*pair)->next;
        if (*pair == 0) {
            Hashmap_Pair *new_pair = arena_make(&arena, 1, sizeof(Hashmap_Pair));
            *new_pair = (Hashmap_Pair) { .frequency = { .word = word, .count = 1 } };
            *pair = new_pair;
            unique_count += 1;
        } else {
            repeat_count += 1;
            (*pair)->frequency.count = clamp_high((*pair)->frequency.count + 1, 0xffff);
        }
    }
    print("[info] Found % unique words, % repeat words\n", fmt(usize, unique_count), fmt(usize, repeat_count));

    // TODO(felix): should have indexed hashmap (w/ ordering option) in base layer!
    Array_Word_Frequency vocabulary = { .arena = &arena };
    usize vocabulary_size = 5000;
    array_ensure_capacity(&vocabulary, vocabulary_size);

    for (usize i = 0; i < vocabulary_size; i += 1) {
        Word_Frequency *best = 0;
        for (usize j = 0; j < word_map.capacity; j += 1) {
            for (Hashmap_Pair *pair = word_map.data[j]; pair != 0; pair = pair->next) {
                if (best != 0 && (pair->frequency.count <= best->count)) continue;
                best = &pair->frequency;
            }
        }
        if (best == 0) break;

        array_push_assume_capacity(&vocabulary, best);
        best->count = 0;
    }
    print("[info] Computed vocabulary of top % most-used words\n", fmt(usize, vocabulary_size));

    for_slice (Document *, document, documents) {
        document->word_vector.arena = &arena;
        array_ensure_capacity(&document->word_vector, vocabulary_size + 1);
        document->word_vector.count = vocabulary_size + 1;

        for_slice (String *, word, document->words) {
            bool unknown = true;
            for (usize i = 0; i < vocabulary.count; i += 1) {
                String vocabulary_word = *vocabulary.data[i].word;
                if (!string_equal(*word, vocabulary_word)) continue;
                document->word_vector.data[i] += 1;
                unknown = false;
                break;
            }
            if (unknown) document->word_vector.data[vocabulary_size] += 1;
        }
    }
    print("[info] Computed word vectors for all % documents\n", fmt(usize, documents.count));

    for_slice (Document *, document, documents) {
        f64 sum_of_squares = 0;
        for_slice (u16 *, frequency, document->word_vector) {
            f64 value = (f64)*frequency;
            sum_of_squares += value * value;
        }
        document->word_vector_length = sqrt(sum_of_squares);
    }
    print("[info] Computed word vector lengths for all % documents\n", fmt(usize, documents.count));

    for (usize i = 0; i < documents.count; i += 1) {
        String bytes = documents.data[i].text;
        usize random_index_from_hash = compute_hash(bytes, documents.count);
        swap(Document, &documents.data[i], &documents.data[random_index_from_hash]);
    }
    print("[info] Shuffled documents using hashes as source of randomness\n");

    enum { train_id, validation_id, testing_id, split_count };
    typedef Slice(f32) Slice_f32;
    Slice_f32 fractions = slice_from_c_array(((f32[]){ [train_id] = 0.6f, [validation_id] = 0.2f, [testing_id] = 0.2f }));

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

    typedef Slice(usize) Slice_usize;
    Slice_usize k_values = slice_of(usize, 1, 3, 5, 7, 9);

    typedef Array(Array_String) Array_Array_String;
    Array_Array_String validation_guesses_per_k = { .arena = &arena };
    array_ensure_capacity(&validation_guesses_per_k, k_values.count);
    validation_guesses_per_k.count = k_values.count;

    for_slice(Array_String *, validation_guesses, validation_guesses_per_k) {
        validation_guesses->arena = &arena;
        array_ensure_capacity(validation_guesses, validation_set.count);
    }

    for_slice (Document *, query, validation_set) {
        typedef Array(f64) Array_f64;
        Array_f64 similarities = { .arena = &arena };
        array_ensure_capacity(&similarities, training_set.count);
        for_slice (Document *, sample, training_set) {
            f64 dot_product = 0;
            for (usize i = 0; i < sample->word_vector.count; i += 1) {
                f64 query_feature = (f64)query->word_vector.data[i];
                f64 sample_feature = (f64)sample->word_vector.data[i];
                dot_product += query_feature * sample_feature;
            }

            f64 cosine_similarity = dot_product / (query->word_vector_length * sample->word_vector_length);
            array_push_assume_capacity(&similarities, &cosine_similarity);
        }

        for (usize k_index = 0; k_index < k_values.count; k_index += 1) {
            usize k = k_values.data[k_index];
            Array_Document k_nearest = { .arena = &arena };
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
            Array_Guess k_guesses = { .arena = &arena };
            array_ensure_capacity(&k_guesses, k);

            for_slice (Document *, neighbour, k_nearest) {
                Guess *existing_guess = 0;
                for_slice (Guess *, previous_guess, k_guesses) {
                    if (!string_equal(previous_guess->label, neighbour->label)) continue;
                    existing_guess = previous_guess;
                    break;
                }

                if (existing_guess != 0) {
                    existing_guess->count += 1;
                } else {
                    Guess new_guess = { .label = neighbour->label, .count = 1 };
                    array_push_assume_capacity(&k_guesses, &new_guess);
                }
            }

            #if BUILD_DEBUG
                usize count = 0;
                for_slice (Guess *, guess, k_guesses) count += guess->count;
                assert(count == k);
            #endif

            Guess winning_guess = {0};
            for_slice (Guess *, guess, k_guesses) {
                if (guess->count > winning_guess.count) winning_guess = *guess;
            }
            array_push_assume_capacity(&validation_guesses_per_k.data[k_index], &winning_guess.label);
        }
    }

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
        f32 percentage = (f32)correct_guess_count / (f32)validation_guesses.count * 100.f;
        print("[info] k = % had accuracy %%\n", fmt(usize, k), fmt(f32, percentage), fmt(char, '%'));
    }

    for (usize k_index = 0; k_index + 1 < k_values.count; k_index += 1) {
        bool is_some_difference_with_next_k = false;
        Array_String guesses = validation_guesses_per_k.data[k_index];
        Array_String next_guesses = validation_guesses_per_k.data[k_index + 1];
        assert(guesses.count == next_guesses.count);
        for (usize i = 0; i < guesses.count; i += 1) {
            if (string_equal(guesses.data[i], next_guesses.data[i])) continue;
            is_some_difference_with_next_k = true;
            break;
        }
        assert(is_some_difference_with_next_k);
    }

    arena_deinit(&arena);
    exit(0);
}
