//
// Created by goran on 02/08/25.
//

#include "LoopScheduler.hpp"

namespace e5 {
    void LoopScheduler::setEntry(const std::string& key, uint32_t interval) {
        auto result = loop_tasks.try_emplace(key, Entry{interval, 0});
        if (!result.second) {
            // If the key already exists, update the interval
            result.first->second.interval = interval;
            result.first->second.counter = 0; // Reset counter for existing entry
        }
    }
    bool LoopScheduler::timeToRun(const std::string& key) {
        auto it = loop_tasks.find(key);
        if (it != loop_tasks.end()) {
            if (it->second.counter >= it->second.interval) {
                it->second.counter = 0; // Reset counter after running
                return true; // Time to run the task
            }
            it->second.counter++;
        }
        return false; // Not time to run the task yet
    }
} // namespace e5