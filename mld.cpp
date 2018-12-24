///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "mld2.h"

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

// you could avoid any allocations by doing the following
//  1 - pre-allocate your output buffer.
//  2 - pre-allocate a bunch of void *'s the get the stack trace

namespace mld
{
//static char mouldy_out[PAGE_SIZE];
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

// Resolve symbol name and source location given the path to the executable and an address

// rename "atos"
int atos(char const * const /*program_name*/, void const * const addr, char * buff, size_t buffSize )
{
    // on stack - probably not safe *meh*
    char addr2line_cmd[512] = {0};

    // ok, instead of PID maybe executable file would let me get additional
    // info - maybe not -

    // yeah, just like this. if this doesn't work then we'll try
    //
    pid_t pid = getpid();
    // we must get "fullPath" but we can still make it relative to the program
    // binary, although, maybe we don't (I'd just trim the last 2 or 3 entries TBH
    sprintf(addr2line_cmd, "atos -p%d -fullPath %p", pid,  addr);//program_name, addr );
    //        sprintf( addr2line_cmd, "atos -d -o %.256s %p", program_name, addr );
    return execute( addr2line_cmd, buff, buffSize );
}

// where do you want it left?
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
                , &address))
    {       // this is how real brackets intermingle. got back and tag a line. (that's all this can do - maybe labels ;-)!). the bracket doesn't really do anything except force starts and ends.. but here it is...
        //            here << "YODDDDDLE!" << stackLevel << filename << address;
        *addr = address;
        return true;
    } else {
        return false;
        //here << "parse failure : " << messages[i];
    }
    // what do you want back? address or nothing - that's it
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
        // what output did atos give?
        here << "failed:atos" << mouldy_buffer;
        return false;
    }
    //    here << "parse_failure:" << mouldy_buffer;
    return false;
    // you may well fail - in this case all you can do is output the message line - which will probably not be
    // clickable unless you want to end up back here (this is possibly desirable)
}

// Print stack trace.
void parseStackTrace( )
{
    int trace_size = 0;
    char ** bts = ( char ** )NULL;

    static const size_t kMaxStackFrames = 64;
    static void * stack_traces[kMaxStackFrames];    // this is now possible. or, frankly, limit yourself - can't read in blocks anyway
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
        // i don't get this. i mean, really? this could easily fail on us but everything we want to know
        // should be right on the stack... g'z.
        free( bts );
    }
}
#if 0
#define name_table_init(x)

struct name_table {
    unsigned nc;
    const char * names[];
};
#endif
// each of these can do followed by another table I guess

// these should contain 2 pointers, the string name and then an onward pointer to the sub objects

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
#if 0
// there's a lot of these in
const char * sigill_names[] = {
    #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    "ILL_NOOP"        /* if only I knew... */
    #else
    "INVALID_0"
    #endif
    , "ILL_ILLOPC"      /* [XSI] illegal opcode */
    , "ILL_ILLTRP"      /* [XSI] illegal trap */
    , "ILL_PRVOPC"      /* [XSI] privileged opcode */
    , "ILL_ILLOPN"      /* [XSI] illegal operand -NOTIMP */
    , "ILL_ILLADR"      /* [XSI] illegal addressing mode -NOTIMP */
    , "ILL_PRVREG"      /* [XSI] privileged register -NOTIMP */
    , "ILL_COPROC"      /* [XSI] coprocessor error -NOTIMP */
    , "ILL_BADSTK"      /* [XSI] internal stack error -NOTIMP */
};

const char * sigfpe_names[] = {
    #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
    "FPE_NOOP"        /* if only I knew... */
    #else
    "INVALID_0"
    #endif
    , "FPE_FLTDIV"      /* [XSI] floating point divide by zero */
    , "FPE_FLTOVF"      /* [XSI] floating point overflow */
    , "FPE_FLTUND"      /* [XSI] floating point underflow */
    , "FPE_FLTRES"      /* [XSI] floating point inexact result */
    , "FPE_FLTINV"      /* [XSI] invalid floating point operation */
    , "FPE_FLTSUB"      /* [XSI] subscript out of range -NOTIMP */
    , "FPE_INTDIV"      /* [XSI] integer divide by zero */
    , "FPE_INTOVF"      /* [XSI] integer overflow */
};
#endif
// at this point I give up!!

// should be "action" if you look at how it's coded up
void signalAction( int sig, siginfo_t * /*siginfo*/, void * /*context*/ )
{
    if (sig == SIGPROF) {
        here << "SHOULD NOT BE HERE!!!";
    } else {
        // check out the sigprof stuff plz
        // for really good diagnostics. *sigh*
        if (sig >= 0 && sig <= int(sizeof(sig_names) / sizeof(sig_names[0]))) {
            here << "Signal caught " << sig_names[sig];
        } else {
            here << "Unknown signal caught " << sig;
        }
    }
    parseStackTrace();

    // WTF?
    //    fflush(stderr);
    //    fflush(stdout);

    _exit(1);
}

void signalActionProf( int /*sig*/, siginfo_t * /*siginfo*/, void * /*context*/ )
{
    here << "signalling profile";
}


// dude, you can even install your own profiler handler - totally awesome - I mean incredible
// I can see this becoming very cool.

// you know, i just don't care about this.
void init()
{
    here;

    struct sigaction sig_action = {};
    sig_action.sa_sigaction = signalAction;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sig_action, NULL);
    sigaction(SIGBUS, &sig_action, NULL);
    sigaction(SIGFPE, &sig_action, NULL);
    sigaction(SIGINT, &sig_action, NULL);
    sigaction(SIGILL, &sig_action, NULL);
    sigaction(SIGTERM, &sig_action, NULL);
    sigaction(SIGABRT, &sig_action, NULL);
    // don't do this! use your own handler!!!


    struct sigaction sig_action_prof = {};
    memset(&sig_action_prof, 0, sizeof(sig_action_prof));
    sig_action_prof.sa_sigaction = signalActionProf;
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


    //}
}
}
