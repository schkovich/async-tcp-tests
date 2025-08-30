// filepath: /home/goran/CLionProjects/pico-sdk-tests/include/TcpAckHandler.hpp
/**
 * @file TcpAckHandler.hpp
 * @brief PerpetualBridge-based handler for ACK events carrying len payload.
 */

#pragma once

#include "PerpetualBridge.hpp"
#include "TcpClient.hpp"
#include <cstdint>

namespace e5 {

using namespace async_tcp;

class TcpAckHandler final : public PerpetualBridge {
        TcpClient &m_io;           ///< TCP client reference
        uint16_t m_len = 0;        ///< Last ACK length delivered via workload

    protected:
        void onWork() override;

    public:
        TcpAckHandler(const AsyncCtx &ctx, TcpClient &io)
            : PerpetualBridge(ctx), m_io(io) {}

        // Accept ACK length via EventBridge workload; takes ownership of payload
        void workload(void *data) override {
            if (data) {
                auto *len_ptr = static_cast<uint16_t *>(data);
                m_len = *len_ptr;
                delete len_ptr; // free payload after copying
            }
        }
};

} // namespace e5

