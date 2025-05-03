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

    structdef(Document) { String label, text; };
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
        while (i < corpus.count && corpus.data[i] != '\r' && corpus.data[i] != '\n') i += 1;
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

            String to_push = string_range(text, start_index_including_whitespace, j);
            array_push_slice_assume_capacity(&text_no_stopwords, &to_push);
        }
        document.text = bit_cast(String) text_no_stopwords;

        array_push(&documents, &document);
    }
    print("[info] Removed % non-ASCII characters and % stopwords\n", fmt(usize, character_removal_count), fmt(usize, stopword_removal_count));

    arena_deinit(&arena);
    exit(0);
}
