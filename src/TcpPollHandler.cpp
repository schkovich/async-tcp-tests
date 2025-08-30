#include "TcpPollHandler.hpp"

namespace e5 {

void TcpPollHandler::onWork() {
    if (auto *writer = m_io.getWriter()) {
        if (writer->hasTimedOut()) {
            writer->onWriteTimeout();
        }
    }
}

} // namespace e5
