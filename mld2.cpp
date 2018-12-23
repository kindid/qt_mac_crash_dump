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

namespace mld
{
static char mouldy_out[PAGE_SIZE];
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
    // yeah, just like this. if this doesn't work then we'll try
    //
    pid_t pid = getpid();
    // we must get "fullPath" but we can still make it relative to the program
    // binary, although, maybe we don't (I'd just trim the last 2 or 3 entries TBH
    sprintf(addr2line_cmd, "atos -p%d -fullPath %p", pid,  addr);//program_name, addr );
    //        sprintf( addr2line_cmd, "atos -d -o %.256s %p", program_name, addr );
    return execute( addr2line_cmd, buff, buffSize );
}

// Check if file exists.
bool FileExists( const char * filename )
{
    if ( FILE * fh = fopen( filename, "r" ) )
    {
        fclose( fh );
        return true;
    }

    return false;
}

// where do you want it left?
bool ParseBacktraceMessage(const char * backtrace_symbol, uintptr_t * addr)
{
    int stackLevel;
    char filename[ 512 ];
    uintptr_t address;

    if (
            sscanf(
                backtrace_symbol
                , "%d%*[ \t]%s%*[ \t]%" SCNxPTR
                , &stackLevel
                , filename
                , &address)) {       // this is how real brackets intermingle. got back and tag a line. (that's all this can do - maybe labels ;-)!)
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
    if (atos(nullptr, (void *) (addr), mouldy_buffer, sizeof(mouldy_buffer)))
    {
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
        here << "ATOS FAILED";
        return false;
    }
//    here << "parse_failure:" << mouldy_buffer;
    return false;
    // you may well fail - in this case all you can do is output the message line - which will probably not be
    // clickable unless you want to end up back here (this is possibly desirable)
}

// Print stack trace.
void PrintStackTrace( )
{
    int trace_size = 0;
    char ** bts = ( char ** )NULL;

    static const size_t kMaxStackFrames = 64;
    static void * stack_traces[kMaxStackFrames];    // this is now possible. or, frankly, limit yourself - can't read in blocks anyway
    trace_size = backtrace(stack_traces, kMaxStackFrames);
    bts = backtrace_symbols(stack_traces, trace_size);

    for ( int i = 0; i < trace_size; ++i )
    {

//        here << "$$$" << messages[i];
        //        int stackLevel;
        //        char filename[ 512 ];
        uintptr_t address;
        //        char symbol[ 512 ];
        //        uintptr_t symbolOffset;
        //        uintptr_t functionOffset;
        //        bool symbolOffsetValid = false;
        //        bool somethingValid = true;

        if (ParseBacktraceMessage(bts[i], &address)) {
            if (parseAtos(address)) {
                // all done, proper output formed
            } else {
                // now it gets a bit mad
                here << "atos_failed:backtrace_symbol:" << bts[i];
            }
        } else {
            here << "parse_fail:backtrace_symbol:" << bts[i];
        }
    }
    // i don't get this. i mean, really? this could easily fail on us but everything we want to know
    // should be right on the stack... g'z.
    if (bts)
    {
        free( bts );
    }
}

#if 0

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGABRT	6	/* abort() */
#if  (defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE))
#define	SIGPOLL	7	/* pollable event ([XSR] generated, not supported) */
#else	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#define	SIGIOT	SIGABRT	/* compatibility */
#define	SIGEMT	7	/* EMT instruction */
#endif	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define	SIGIO	23	/* input/output possible signal */
#endif
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define SIGWINCH 28	/* window size changes */
#define SIGINFO	29	/* information request */
#endif
#define SIGUSR1 30	/* user defined signal 1 */
#define SIGUSR2 31	/* user defined signal 2 */
#endif


// should be "action" if you look at how it's coded up
void Handler( int sig, siginfo_t * siginfo, void * /*context*/ )
{
    // dude, there's a HUGE number
    // output with "here" so you can easily see that is was
    // the SignalHandler::handler() that caused it, make the
    // report clickable & be able to
    switch(sig)
    {
    case SIGSEGV:
        fputs("Caught SIGSEGV: Segmentation Fault\n", stderr);
        break;

    case SIGBUS:
        fputs("Caught SIGBUG: Bus error (bad memory access)\n", stderr);
        break;

    case SIGINT:
        fputs("Caught SIGINT: Interactive attention signal, (usually ctrl+c)\n", stderr);
        break;

    case SIGFPE:
        switch(siginfo->si_code)
        {
        case FPE_INTDIV:
            fputs("Caught SIGFPE: (integer divide by zero)\n", stderr);
            break;
        case FPE_INTOVF:
            fputs("Caught SIGFPE: (integer overflow)\n", stderr);
            break;
        case FPE_FLTDIV:
            fputs("Caught SIGFPE: (floating-point divide by zero)\n", stderr);
            break;
        case FPE_FLTOVF:
            fputs("Caught SIGFPE: (floating-point overflow)\n", stderr);
            break;
        case FPE_FLTUND:
            fputs("Caught SIGFPE: (floating-point underflow)\n", stderr);
            break;
        case FPE_FLTRES:
            fputs("Caught SIGFPE: (floating-point inexact result)\n", stderr);
            break;
        case FPE_FLTINV:
            fputs("Caught SIGFPE: (floating-point invalid operation)\n", stderr);
            break;
        case FPE_FLTSUB:
            fputs("Caught SIGFPE: (subscript out of range)\n", stderr);
            break;
        default:
            fputs("Caught SIGFPE: Arithmetic Exception\n", stderr);
            break;
        }
        break;

    case SIGILL:
        switch(siginfo->si_code)
        {
        case ILL_ILLOPC:
            fputs("Caught SIGILL: (illegal opcode)\n", stderr);
            break;
        case ILL_ILLOPN:
            fputs("Caught SIGILL: (illegal operand)\n", stderr);
            break;
        case ILL_ILLADR:
            fputs("Caught SIGILL: (illegal addressing mode)\n", stderr);
            break;
        case ILL_ILLTRP:
            fputs("Caught SIGILL: (illegal trap)\n", stderr);
            break;
        case ILL_PRVOPC:
            fputs("Caught SIGILL: (privileged opcode)\n", stderr);
            break;
        case ILL_PRVREG:
            fputs("Caught SIGILL: (privileged register)\n", stderr);
            break;
        case ILL_COPROC:
            fputs("Caught SIGILL: (coprocessor error)\n", stderr);
            break;
        case ILL_BADSTK:
            fputs("Caught SIGILL: (internal stack error)\n", stderr);
            break;
        default:
            fputs("Caught SIGILL: Illegal Instruction\n", stderr);
            break;
        }
        break;

    case SIGTERM:
        fputs("Caught SIGTERM: a termination request was sent to the program\n", stderr);
        break;
    case SIGABRT:
        fputs("Caught SIGABRT: usually caused by an abort() or assert()\n", stderr);
        break;
    default:
        break;
    }
    PrintStackTrace( );
    fflush( stderr );
    fflush( stdout );

    _exit( 1 );
}

// you know, i just don't care about this.
bool init()
{
#if 0
    char path[ 512 ];
    uint32_t size = sizeof( path );
    if ( _NSGetExecutablePath( path, &size ) == 0 )
    {
        if ( ! realpath( path, m_ExeFilename ) )
        {
            strcpy( m_ExeFilename, path );
        }
    }
    else
    {
        strcpy( m_ExeFilename, argv ? argv : "" );
    }
#endif
    struct sigaction sig_action = {};
    sig_action.sa_sigaction = Handler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = SA_SIGINFO;

    // just why. it's C++ code - should probably use Qt
    // QList<int> or something. then just for_each
    int toCatch[ ] = {
        SIGSEGV,
        SIGBUS,
        SIGFPE,
        SIGINT,
        SIGILL,
        SIGTERM,
        SIGABRT
    };

    bool okay = true;
#define PiArraySize(x) (sizeof(toCatch) / sizeof(toCatch[0]))
    for ( size_t toCatchIx = 0; toCatchIx < PiArraySize( toCatch ); ++toCatchIx )
    {
        okay &= sigaction( toCatch[ toCatchIx ], &sig_action, NULL ) == 0;
    }

    return okay;
}
}
