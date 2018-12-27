///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <mld.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <mach-o/dyld.h>
#include <here.h>
#include <sys/time.h>

struct mld_ctor {
    mld_ctor() { mld::init(); }
};

static mld_ctor init;

namespace mld
{
static char mouldy_buffer[PAGE_SIZE];

int execute( const char * cmd, char * buf, size_t bufSize ) {
    if (FILE * pipe_cmd = popen(cmd, "r")) {
        while (fgets(buf, bufSize, pipe_cmd) != NULL) {
        }
        pclose( pipe_cmd );
        return 1;
    }
    return 0;
}

int atos(char const * const /*program_name*/, void const * const addr, char * buff, size_t buffSize )
{
    char addr2line_cmd[512] = {0};
    pid_t pid = getpid();
    sprintf(addr2line_cmd, "atos -p%d -fullPath %p", pid,  addr);//program_name, addr );
    return execute( addr2line_cmd, buff, buffSize );
}

bool parseBacktraceMessage(const char * backtrace_symbol, uintptr_t * addr)
{
    int stackLevel;
    char filename[ 512 ];
    uintptr_t address;

    if (sscanf(
            backtrace_symbol
            , "%d%*[ \t]%s%*[ \t]%" SCNxPTR
            , &stackLevel
            , filename
            , &address)) {
        *addr = address;
        return true;
    } else {
        return false;
    }
}

bool parseAtos(uintptr_t addr)
{
    mouldy_buffer[0] = '\0';
    // this function must \0 postifx or
    if (atos(nullptr, (void *) (addr), mouldy_buffer, sizeof(mouldy_buffer))) {
        //        here << "!!!" << mouldy_buffer;
        char * line_start = nullptr;
        char * line_end = nullptr;
        char * file_start = nullptr;
        char * file_end = nullptr;
        // dude, you even KNOW how many bytes came back... g'zus
        char * last = mouldy_buffer + strlen(mouldy_buffer);
        // scan back until the first ')'
        while (last >= mouldy_buffer && *last != ')') {
            last--;
        }
        if (last < mouldy_buffer) {
            // deaded
            //            here << "parse failure";
        } else {
            last--;
            line_end = last;
            while (last >= mouldy_buffer && *last != ':') {
                last--;
            }
            if (last < mouldy_buffer) {
                // deaded
                //                here << "parse failure";
            } else {
                line_start = last + 1;
                last--;
                if (last < mouldy_buffer) {
                    //                    here << "parse failure";
                } else {
                    file_end = last;
                    //here << *file_end;
                    while (last >= mouldy_buffer && *last != '(') {
                        last--;
                    }
                    if (last < mouldy_buffer) {
                        //                        here << "parse failure";
                    } else {
                        file_start = last + 1;
                        // this, right here, is how you get performance.
                        //                                    here << "YOU DID IT!";
                        // i don't want quotes dude.
                        // the message is jsut 'buff' the first part of which you want anyway
                        //                        here << "THIS->";
                        qDebug().nospace().noquote() << "file:///" << QLatin1String(file_start, (file_end - file_start) + 1) << ":" << QLatin1String(line_start, 1 + (line_end - line_start)) << ": " << QLatin1String(mouldy_buffer, file_start - 1);//this is my message";
                        return true;
                        //here << QLatin1String(line_start, line_end - line_start);
                    }
                }
            }
        }
    } else {
        here << "failed:atos" << mouldy_buffer;
        return false;
    }
    return false;
}

void dump_stack_trace( )
{
    int trace_size = 0;
    char ** bts = ( char ** )NULL;

    static const size_t kMaxStackFrames = 64;
    static void * stack_traces[kMaxStackFrames];
    trace_size = backtrace(stack_traces, kMaxStackFrames);
    bts = backtrace_symbols(stack_traces, trace_size);
    if (bts) {
        for ( int i = 0; i < trace_size; ++i) {
            uintptr_t address;
            if (parseBacktraceMessage(bts[i], &address)) {
                if (parseAtos(address)) {
                    // all done, proper output formed
                } else {
                    // todo; there could be return data in "buff"
                    here << "atos_failed:backtrace_symbol:" << bts[i];
                }
            } else {
                here << "parse_fail:backtrace_symbol:" << bts[i];
            }
        }
        free( bts );
    }
}

const char * sig_names[] =
{
    "INVALID_0"       // 0
    , "SIGHUP"          // 1
    , "SIGINT"          // 2
    , "SIGQUIT"         // 3
    , "SIGILL"          // 4
    , "SIGTRAP"         // 5
    , "SIGABRT"         // 6
    #if  (defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE))
    , "SIGPOLL"         // 7
    #else
    , "SIGEMT"          // 7
    #endif
    , "SIGFPE"          // 8
    , "SIGKILL"         // 9
    , "SIGBUS"          // 10
    , "SIGSEGV"         // 11
    , "SIGSYS"          // 12
    , "SIGPIPE"         // 13
    , "SIGALRM"         // 14
    , "SIGTERM"         // 15
    , "SIGURG"          // 16
    , "SIGSTOP"         // 17
    , "SIGTSTP"         // 18
    , "SIGCONT"         // 19
    , "SIGCHLD"         // 20
    , "SIGTTIN"         // 21
    , "SIGTTOU"         // 22
    #if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
    , "SIGIO"           // 23
    #else
    , "INVALID_23"      // 23
    #endif
    , "SIGXCPU"         // 24
    , "SIGXFSZ"         // 25
    , "SIGVTALRM"       // 26
    , "SIGPROF"         // 27   // this one seems really handy.
    #if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
    , "SIGWINCH"        // 28
    , "SIGINFO"         // 29
    #else
    , "INVALID_28"      // 28
    , "INVALID_29"      // 29
    #endif
    , "SIGUSR1"         // 30
    , "SIGUSR2"         // 31
};

void sig_handler_generic( int sig, siginfo_t * /*siginfo*/, void * /*context*/ )
{
    if (sig == SIGPROF) {
        here << "SHOULD NOT BE HERE!!!";
    } else {
        // check out the sigprof stuff plz
        // for really good diagnostics. *sigh*
        if (sig >= 0 && sig <= int(sizeof(sig_names) / sizeof(sig_names[0]))) {
            here << "////////////////////////////////////////////////////////////////";
            here << "Signal caught " << sig_names[sig];
        } else {
            here << "Unknown signal caught " << sig;
        }
    }
    dump_stack_trace();
    here << "////////////////////////////////////////////////////////////////";
    _exit(1);
}
#if 0
void sig_handler_prof( int /*sig*/, siginfo_t * /*siginfo*/, void * /*context*/ )
{
    here << "signalling profile";
}
#endif
void init()
{
    here;

    struct sigaction sig_action = {};
    memset(&sig_action, 0, sizeof(sig_action));
    sig_action.sa_sigaction = sig_handler_generic;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sig_action, NULL);
    sigaction(SIGBUS, &sig_action, NULL);
    sigaction(SIGFPE, &sig_action, NULL);
    sigaction(SIGINT, &sig_action, NULL);
    sigaction(SIGILL, &sig_action, NULL);
    sigaction(SIGTERM, &sig_action, NULL);
    sigaction(SIGABRT, &sig_action, NULL);

#if 0
    struct sigaction sig_action_prof = {};
    memset(&sig_action_prof, 0, sizeof(sig_action_prof));
    sig_action_prof.sa_sigaction = sig_handler_prof;
    sigemptyset(&sig_action_prof.sa_mask);
    sig_action_prof.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGPROF, &sig_action_prof, NULL);

    static struct itimerval timer;

    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0; //1000000 / 1000; /* 1000hz */
    timer.it_value = timer.it_interval;

    /* Install timer */
    if (setitimer(ITIMER_PROF, &timer, NULL) != 0)
    {
        printf("Timer could not be initialized \n");
    }
#endif
}
}
