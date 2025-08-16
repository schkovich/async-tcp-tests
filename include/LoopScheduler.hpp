//
// Created by goran on 02/08/25.
//
#pragma once

#include <cstdint>
#include <unordered_map>

namespace e5 {

    using namespace std;
    using entry_key = uint8_t;

    class LoopScheduler {
        struct Entry {
                uint32_t interval;
                uint32_t counter;
        };

        unordered_map<entry_key , Entry> loop_tasks;

    public:
        LoopScheduler() = default;
        ~LoopScheduler() = default;
        void setEntry(const uint8_t &key, uint32_t interval);
        bool timeToRun(const uint8_t &key);
    };

} // namespace e5
