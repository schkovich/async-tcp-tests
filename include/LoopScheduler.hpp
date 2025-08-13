//
// Created by goran on 02/08/25.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace e5 {

    using namespace std;
    using entry_key = std::string;

    class LoopScheduler {
        struct Entry {
                uint32_t interval;
                uint32_t counter;
        };

        unordered_map<entry_key , Entry> loop_tasks;

    public:
        LoopScheduler() = default;
        ~LoopScheduler() = default;
        void setEntry(const std::string &key, uint32_t interval);
        bool timeToRun(const std::string &key);
    };

} // namespace e5
