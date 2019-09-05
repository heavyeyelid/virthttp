#pragma once
#include "virt_wrap/enums/Base.hpp"
#include "virt_wrap/utility.hpp"
#include <libvirt/libvirt-domain.h>

namespace virt {

class Domain::ProcessSignal : private VirtEnumStorage<virDomainProcessSignal>, public VirtEnumBase<ProcessSignal>, public EnumHelper<ProcessSignal> {
    friend VirtEnumBase<ProcessSignal>;
    friend EnumHelper<ProcessSignal>;
    enum class Underlying {
        NOP = VIR_DOMAIN_PROCESS_SIGNAL_NOP,   /* No constant in POSIX/Linux */
        HUP = VIR_DOMAIN_PROCESS_SIGNAL_HUP,   /* SIGHUP */
        INT = VIR_DOMAIN_PROCESS_SIGNAL_INT,   /* SIGINT */
        QUIT = VIR_DOMAIN_PROCESS_SIGNAL_QUIT, /* SIGQUIT */
        ILL = VIR_DOMAIN_PROCESS_SIGNAL_ILL,   /* SIGILL */
        TRAP = VIR_DOMAIN_PROCESS_SIGNAL_TRAP, /* SIGTRAP */
        ABRT = VIR_DOMAIN_PROCESS_SIGNAL_ABRT, /* SIGABRT */
        BUS = VIR_DOMAIN_PROCESS_SIGNAL_BUS,   /* SIGBUS */
        FPE = VIR_DOMAIN_PROCESS_SIGNAL_FPE,   /* SIGFPE */
        KILL = VIR_DOMAIN_PROCESS_SIGNAL_KILL, /* SIGKILL */

        USR1 = VIR_DOMAIN_PROCESS_SIGNAL_USR1,     /* SIGUSR1 */
        SEGV = VIR_DOMAIN_PROCESS_SIGNAL_SEGV,     /* SIGSEGV */
        USR2 = VIR_DOMAIN_PROCESS_SIGNAL_USR2,     /* SIGUSR2 */
        PIPE = VIR_DOMAIN_PROCESS_SIGNAL_PIPE,     /* SIGPIPE */
        ALRM = VIR_DOMAIN_PROCESS_SIGNAL_ALRM,     /* SIGALRM */
        TERM = VIR_DOMAIN_PROCESS_SIGNAL_TERM,     /* SIGTERM */
        STKFLT = VIR_DOMAIN_PROCESS_SIGNAL_STKFLT, /* Not in POSIX (SIGSTKFLT on Linux )*/
        CHLD = VIR_DOMAIN_PROCESS_SIGNAL_CHLD,     /* SIGCHLD */
        CONT = VIR_DOMAIN_PROCESS_SIGNAL_CONT,     /* SIGCONT */
        STOP = VIR_DOMAIN_PROCESS_SIGNAL_STOP,     /* SIGSTOP */

        TSTP = VIR_DOMAIN_PROCESS_SIGNAL_TSTP,     /* SIGTSTP */
        TTIN = VIR_DOMAIN_PROCESS_SIGNAL_TTIN,     /* SIGTTIN */
        TTOU = VIR_DOMAIN_PROCESS_SIGNAL_TTOU,     /* SIGTTOU */
        URG = VIR_DOMAIN_PROCESS_SIGNAL_URG,       /* SIGURG */
        XCPU = VIR_DOMAIN_PROCESS_SIGNAL_XCPU,     /* SIGXCPU */
        XFSZ = VIR_DOMAIN_PROCESS_SIGNAL_XFSZ,     /* SIGXFSZ */
        VTALRM = VIR_DOMAIN_PROCESS_SIGNAL_VTALRM, /* SIGVTALRM */
        PROF = VIR_DOMAIN_PROCESS_SIGNAL_PROF,     /* SIGPROF */
        WINCH = VIR_DOMAIN_PROCESS_SIGNAL_WINCH,   /* Not in POSIX (SIGWINCH on Linux) */
        POLL = VIR_DOMAIN_PROCESS_SIGNAL_POLL,     /* SIGPOLL (also known as SIGIO on Linux) */

        PWR = VIR_DOMAIN_PROCESS_SIGNAL_PWR, /* Not in POSIX (SIGPWR on Linux) */
        SYS = VIR_DOMAIN_PROCESS_SIGNAL_SYS, /* SIGSYS (also known as SIGUNUSED on Linux) */
        RT0 = VIR_DOMAIN_PROCESS_SIGNAL_RT0, /* SIGRTMIN */
        RT1 = VIR_DOMAIN_PROCESS_SIGNAL_RT1, /* SIGRTMIN + 1 */
        RT2 = VIR_DOMAIN_PROCESS_SIGNAL_RT2, /* SIGRTMIN + 2 */
        RT3 = VIR_DOMAIN_PROCESS_SIGNAL_RT3, /* SIGRTMIN + 3 */
        RT4 = VIR_DOMAIN_PROCESS_SIGNAL_RT4, /* SIGRTMIN + 4 */
        RT5 = VIR_DOMAIN_PROCESS_SIGNAL_RT5, /* SIGRTMIN + 5 */
        RT6 = VIR_DOMAIN_PROCESS_SIGNAL_RT6, /* SIGRTMIN + 6 */
        RT7 = VIR_DOMAIN_PROCESS_SIGNAL_RT7, /* SIGRTMIN + 7 */

        RT8 = VIR_DOMAIN_PROCESS_SIGNAL_RT8,   /* SIGRTMIN + 8 */
        RT9 = VIR_DOMAIN_PROCESS_SIGNAL_RT9,   /* SIGRTMIN + 9 */
        RT10 = VIR_DOMAIN_PROCESS_SIGNAL_RT10, /* SIGRTMIN + 10 */
        RT11 = VIR_DOMAIN_PROCESS_SIGNAL_RT11, /* SIGRTMIN + 11 */
        RT12 = VIR_DOMAIN_PROCESS_SIGNAL_RT12, /* SIGRTMIN + 12 */
        RT13 = VIR_DOMAIN_PROCESS_SIGNAL_RT13, /* SIGRTMIN + 13 */
        RT14 = VIR_DOMAIN_PROCESS_SIGNAL_RT14, /* SIGRTMIN + 14 */
        RT15 = VIR_DOMAIN_PROCESS_SIGNAL_RT15, /* SIGRTMIN + 15 */
        RT16 = VIR_DOMAIN_PROCESS_SIGNAL_RT16, /* SIGRTMIN + 16 */
        RT17 = VIR_DOMAIN_PROCESS_SIGNAL_RT17, /* SIGRTMIN + 17 */

        RT18 = VIR_DOMAIN_PROCESS_SIGNAL_RT18, /* SIGRTMIN + 18 */
        RT19 = VIR_DOMAIN_PROCESS_SIGNAL_RT19, /* SIGRTMIN + 19 */
        RT20 = VIR_DOMAIN_PROCESS_SIGNAL_RT20, /* SIGRTMIN + 20 */
        RT21 = VIR_DOMAIN_PROCESS_SIGNAL_RT21, /* SIGRTMIN + 21 */
        RT22 = VIR_DOMAIN_PROCESS_SIGNAL_RT22, /* SIGRTMIN + 22 */
        RT23 = VIR_DOMAIN_PROCESS_SIGNAL_RT23, /* SIGRTMIN + 23 */
        RT24 = VIR_DOMAIN_PROCESS_SIGNAL_RT24, /* SIGRTMIN + 24 */
        RT25 = VIR_DOMAIN_PROCESS_SIGNAL_RT25, /* SIGRTMIN + 25 */
        RT26 = VIR_DOMAIN_PROCESS_SIGNAL_RT26, /* SIGRTMIN + 26 */
        RT27 = VIR_DOMAIN_PROCESS_SIGNAL_RT27, /* SIGRTMIN + 27 */

        RT28 = VIR_DOMAIN_PROCESS_SIGNAL_RT28, /* SIGRTMIN + 28 */
        RT29 = VIR_DOMAIN_PROCESS_SIGNAL_RT29, /* SIGRTMIN + 29 */
        RT30 = VIR_DOMAIN_PROCESS_SIGNAL_RT30, /* SIGRTMIN + 30 */
        RT31 = VIR_DOMAIN_PROCESS_SIGNAL_RT31, /* SIGRTMIN + 31 */
        RT32 = VIR_DOMAIN_PROCESS_SIGNAL_RT32, /* SIGRTMIN + 32 / SIGRTMAX */
    } constexpr static default_value{};

  protected:
    constexpr static std::array values = {
        "nop",  "hup",  "int",  "quit", "ill",  "trap", "abrt", "bus",  "fpe",  "kill",   "usr1", "segv",  "usr2", "pipe", "alrm", "term", "stkflt",
        "chld", "cont", "stop", "tstp", "ttin", "ttou", "urg",  "xcpu", "xfsz", "vtalrm", "prof", "winch", "poll", "pwr",  "sys",  "rt0",  "rt1",
        "rt2",  "rt3",  "rt4",  "rt5",  "rt6",  "rt7",  "rt8",  "rt9",  "rt10", "rt11",   "rt12", "rt13",  "rt14", "rt15", "rt16", "rt17", "rt18",
        "rt19", "rt20", "rt21", "rt22", "rt23", "rt24", "rt25", "rt26", "rt27", "rt28",   "rt29", "rt30",  "rt31", "rt32"};

  public:
    using VirtEnumBase::VirtEnumBase;
    constexpr static auto from_string(std::string_view sv) { return EnumHelper{}.from_string_base(sv); }
    // using enum Underlying;
    constexpr static auto NOP = Underlying::NOP;
    constexpr static auto HUP = Underlying::HUP;
    constexpr static auto INT = Underlying::INT;
    constexpr static auto QUIT = Underlying::QUIT;
    constexpr static auto ILL = Underlying::ILL;
    constexpr static auto TRAP = Underlying::TRAP;
    constexpr static auto ABRT = Underlying::ABRT;
    constexpr static auto BUS = Underlying::BUS;
    constexpr static auto FPE = Underlying::FPE;
    constexpr static auto KILL = Underlying::KILL;
    constexpr static auto USR1 = Underlying::USR1;
    constexpr static auto SEGV = Underlying::SEGV;
    constexpr static auto USR2 = Underlying::USR2;
    constexpr static auto PIPE = Underlying::PIPE;
    constexpr static auto ALRM = Underlying::ALRM;
    constexpr static auto TERM = Underlying::TERM;
    constexpr static auto STKFLT = Underlying::STKFLT;
    constexpr static auto CHLD = Underlying::CHLD;
    constexpr static auto CONT = Underlying::CONT;
    constexpr static auto STOP = Underlying::STOP;
    constexpr static auto TSTP = Underlying::TSTP;
    constexpr static auto TTIN = Underlying::TTIN;
    constexpr static auto TTOU = Underlying::TTOU;
    constexpr static auto URG = Underlying::URG;
    constexpr static auto XCPU = Underlying::XCPU;
    constexpr static auto XFSZ = Underlying::XFSZ;
    constexpr static auto VTALRM = Underlying::VTALRM;
    constexpr static auto PROF = Underlying::PROF;
    constexpr static auto WINCH = Underlying::WINCH;
    constexpr static auto POLL = Underlying::POLL;
    constexpr static auto PWR = Underlying::PWR;
    constexpr static auto SYS = Underlying::SYS;
    constexpr static auto RT0 = Underlying::RT0;
    constexpr static auto RT1 = Underlying::RT1;
    constexpr static auto RT2 = Underlying::RT2;
    constexpr static auto RT3 = Underlying::RT3;
    constexpr static auto RT4 = Underlying::RT4;
    constexpr static auto RT5 = Underlying::RT5;
    constexpr static auto RT6 = Underlying::RT6;
    constexpr static auto RT7 = Underlying::RT7;
    constexpr static auto RT8 = Underlying::RT8;
    constexpr static auto RT9 = Underlying::RT9;
    constexpr static auto RT10 = Underlying::RT10;
    constexpr static auto RT11 = Underlying::RT11;
    constexpr static auto RT12 = Underlying::RT12;
    constexpr static auto RT13 = Underlying::RT13;
    constexpr static auto RT14 = Underlying::RT14;
    constexpr static auto RT15 = Underlying::RT15;
    constexpr static auto RT16 = Underlying::RT16;
    constexpr static auto RT17 = Underlying::RT17;
    constexpr static auto RT18 = Underlying::RT18;
    constexpr static auto RT19 = Underlying::RT19;
    constexpr static auto RT20 = Underlying::RT20;
    constexpr static auto RT21 = Underlying::RT21;
    constexpr static auto RT22 = Underlying::RT22;
    constexpr static auto RT23 = Underlying::RT23;
    constexpr static auto RT24 = Underlying::RT24;
    constexpr static auto RT25 = Underlying::RT25;
    constexpr static auto RT26 = Underlying::RT26;
    constexpr static auto RT27 = Underlying::RT27;
    constexpr static auto RT28 = Underlying::RT28;
    constexpr static auto RT29 = Underlying::RT29;
    constexpr static auto RT30 = Underlying::RT30;
    constexpr static auto RT31 = Underlying::RT31;
    constexpr static auto RT32 = Underlying::RT32;
};

}