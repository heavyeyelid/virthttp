#pragma once

enum class SocketProto {
    TCP,
    UDP,
};

enum class TransportProto {
    HTTP1,
    HTTP2,
    HTTP3,
    WebSockets, // least priority
};