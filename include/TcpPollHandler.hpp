#pragma once

#include "PerpetualBridge.hpp"
#include "TcpClient.hpp"
#include "TcpWriter.hpp"

namespace e5 {

    using namespace async_tcp;

    class TcpPollHandler final : public PerpetualBridge {
            TcpClient &m_io; ///< TCP client reference

        protected:
            /**
             * @brief Execute poll work under async-context guarantees.
             *
             * Default behavior mirrors TcpClient's existing poll lambda:
             * checks the writer for timeouts and triggers onWriteTimeout.
             * If a custom action is provided, it takes precedence.
             */
            void onWork() override;

        public:
            /**
             * @brief Construct a TcpPollHandler with default behavior.
             * @param ctx Async context for execution
             * @param io TcpClient reference
             */
            TcpPollHandler(const AsyncCtx &ctx, TcpClient &io)
                : PerpetualBridge(ctx), m_io(io) {}
    };

} // namespace e5
