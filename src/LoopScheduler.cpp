//
// Created by goran on 02/08/25.
//

#include "LoopScheduler.hpp"

namespace e5 {
    void LoopScheduler::setEntry(const uint8_t &key,
                                 const uint32_t interval) {
        if (auto [fst, snd] = loop_tasks.try_emplace(
            key, Entry{interval, 0});
            !snd) {
            // If the key already exists, update the interval
            fst->second.interval = interval;
            fst->second.counter = 0; // Reset counter for existing entry
        }
    }
    bool LoopScheduler::timeToRun(const uint8_t &key) {
        if (const auto it = loop_tasks.find(key); it != loop_tasks.end()) {
            if (it->second.counter >= it->second.interval) {
                it->second.counter = 0; // Reset counter after running
                return true;            // Time to run the task
            }
            it->second.counter++;
        }
        return false; // Not time to run the task yet
    }
} // namespace e5