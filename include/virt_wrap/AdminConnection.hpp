//
// Created by _as on 2019-01-31.
//

#pragma once

#include <gsl/gsl>
#include <libvirt/libvirt-admin.h>
#include "Connection.hpp"

namespace virt {
    class AdminConnection {
        virAdmConnectPtr underlying = nullptr;

    public:
        inline AdminConnection(gsl::czstring<> name, Connection::Flags flags) {
            underlying = virAdmConnectOpen(name,
                                           to_integral(
                                                   flags));
        }

        ~AdminConnection() { virAdmConnectClose(underlying); }

        inline bool isAlive() { return virAdmConnectIsAlive(underlying) != 0; }

        inline gsl::zstring<> getURI() { return virAdmConnectGetURI(underlying); }

        // TODO
    };
}