// filepath: /home/goran/CLionProjects/pico-sdk-tests/src/TcpAckHandler.cpp
#include "TcpAckHandler.hpp"
#include <Arduino.h>

namespace e5 {

void TcpAckHandler::onWork() {
    // Notify writer about ACK if configured
    if (auto *writer = m_io.getWriter()) {
        writer->onAckReceived(m_len);
    }
    DEBUGWIRE("[TcpAckHandler][:i%d] ACK len=%u handled\n", m_io.getClientId(), static_cast<unsigned>(m_len));
}

} // namespace e5
