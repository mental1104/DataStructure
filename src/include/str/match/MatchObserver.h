#pragma once
#include "dsa_string.h"

#include <cstddef>
#include <cstdio>
#include <string_view>

inline void moPrintString(std::string_view value) {
    for (char character : value) {
        std::printf("%4c", character);
    }
}

inline void moPrintIndexLine(std::size_t count) {
    for (std::size_t index = 0; index < count; ++index) {
        std::printf("%4zu", index);
    }
    std::printf("\n");
}

struct MatchObserver {
    virtual ~MatchObserver() = default;
    virtual void onNextTable(std::string_view, const int*, int) {}
    virtual void onProgress(std::string_view, std::string_view, int, int, const int*, int) {}
    virtual void onBCTable(const int*, int) {}
    virtual void onGSTable(const int*, int, std::string_view) {}
    virtual void onKRProgress(std::string_view, std::string_view, std::size_t, long long, long long) {}
    virtual void onPause() {}
};

struct StdoutMatchObserver : MatchObserver {
    int step = 0;

    void onNextTable(std::string_view pattern, const int* next, int length) override {
        if (next == nullptr) {
            return;
        }
        moPrintString(pattern);
        std::printf("\n");
        moPrintIndexLine(static_cast<std::size_t>(length));
        for (int index = 0; index < length; ++index) {
            std::printf("%4d", next[index]);
        }
        std::printf("\n\n");
    }

    void onProgress(std::string_view text,
                    std::string_view pattern,
                    int alignment,
                    int pattern_index,
                    const int*,
                    int) override {
        std::printf("\n-- Step %2d: --\n", ++step);
        moPrintIndexLine(text.size());
        moPrintString(text);
        std::printf("\n");
        if (alignment + pattern_index >= 0) {
            for (int index = 0; index < alignment + pattern_index; ++index) {
                std::printf("%4c", ' ');
            }
            std::printf("%4c", '|');
        }
        std::printf("\n");
        for (int index = 0; index < alignment; ++index) {
            std::printf("%4c", ' ');
        }
        moPrintString(pattern);
        std::printf("\n");
    }

    void onBCTable(const int* bad_character, int length) override {
        std::printf("\n-- bc[] Table ---------------\n");
        for (int index = 0; index < length; ++index) {
            if (bad_character[index] >= 0) {
                std::printf("%4c", static_cast<char>(index));
            }
        }
        std::printf("\n");
        for (int index = 0; index < length; ++index) {
            if (bad_character[index] >= 0) {
                std::printf("%4d", bad_character[index]);
            }
        }
        std::printf("\n\n");
    }

    void onGSTable(const int* good_suffix, int length, std::string_view pattern) override {
        std::printf("-- gs[] Table ---------------\n");
        moPrintIndexLine(static_cast<std::size_t>(length));
        moPrintString(pattern);
        std::printf("\n");
        for (int index = 0; index < length; ++index) {
            std::printf("%4d", good_suffix[index]);
        }
        std::printf("\n\n");
    }

    void onKRProgress(std::string_view text,
                      std::string_view pattern,
                      std::size_t offset,
                      long long pattern_hash,
                      long long text_hash) override {
        std::printf("\n-- Step %2d (k=%zu) --\n", ++step, offset);
        moPrintIndexLine(text.size());
        moPrintString(text);
        std::printf("\n");
        for (std::size_t index = 0; index < offset; ++index) {
            std::printf("%4c", ' ');
        }
        moPrintString(pattern);
        std::printf("\n");
        std::printf("hashP=%lld, hashT=%lld\n", pattern_hash, text_hash);
    }

    void onPause() override { std::getchar(); }
};

struct NoopMatchObserver : MatchObserver {};
