#pragma once
#include <array>
#include <string_view>
#include "cexpr_algs.hpp"

using namespace std::literals;

class ErrorMessages {
    using P = std::pair<int, std::string_view>;
    constexpr static std::array pairs = {
        P{-1, "Unsupported"sv},
        P{0, "Syntax error"sv},
        P{1, "Bad X-Auth-Key"sv},
        P{2, "Bad URL"sv},
        P{3, "Bad request"sv},
        P{4, "No subsystem"sv},
        P{5, "Bad subsystem"sv},
        P{6, "Subsystem requires parameters"},
        P{7, "Bad subsystem parameter"},
        P{10, "Failed to open connection to the libvirt daemon"sv},
        P{100, "Bad object identifier"sv},
        P{101, "Invalid search key"sv},
        P{102, "Search flag doesn't exist"sv},
        P{105, "Failed to create domain"sv},
        P{122, "Bad action"sv},
        P{123, "Unknown action"sv},
        P{200, "Could not shut down the domain"sv},
        P{201, "Domain is not running"sv},
        P{202, "Could not start the domain"sv},
        P{203, "Domain is already active"sv},
        P{204, "No status actions specified"sv},
        P{205, "Renaming failed"sv},
        P{206, "Setting available memory failed"sv},
        P{207, "Setting maximum available memory failed"sv},
        P{208, "Setting autostart policy failed"sv},
        P{209, "Could not destroy the domain"sv},
        P{210, "Domain is not active"sv},
        P{211, "Domain is not suspended"sv},
        P{212, "Cannot resume the domain"sv},
        P{213, "Cannot reboot the domain"sv},
        P{214, "Cannot reset the domain"sv},
        P{215, "Cannot suspend the domain"sv},
        P{216, "Cannot delete the domain"sv},
        P{298, "PATCH data has to be an array of actions"sv},
        P{300, "Invalid power management value"sv},
        P{301, "Invalid flag"sv},
        P{500, "Error occurred while getting network status"sv},
        P{503, "Error occurred while getting network autostart"sv},
    };

  public:
    constexpr std::string_view operator[](int errc) const noexcept {
        const auto it = cexpr::find_if(pairs.begin(), pairs.end(), [=](auto p) { return p.first == errc; });
        return it != pairs.end() ? it->second : "Unknown error"sv;
    }
} constexpr inline static error_messages;