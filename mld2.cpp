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

namespace ExceptionHandler
{
    // remove this completely.
    char m_ExeFilename[ 512 ];

    // Execute cmd store stdout into buf (up to bufSize).
    int Execute( const char * cmd, char * buf, size_t bufSize )
    {
//        char filename[ 512 ];
//        sprintf( filename, "%d.tmp", rand( ) );

//        here << "EXECUTE";

//        if ( FILE * file = fopen( filename, "w" ) )
//        {
            if ( FILE * ptr = popen( cmd, "r" ) )
            {
                // just weird...
                // just read one line
                while ( fgets( buf, bufSize, ptr ) != NULL )
                {
                    // format... a valid c or c++ name
                    //  this will be demented
                    //  if I were you I'd parse backwards!!!!
                    // last ) then read an int until you hit a :
                    // from there keep reading backwards until you get to
                    //  to a (. yes, this COULD be a problem...
                    //   but there's a limit
                    //
//                    here << buf;
                    // why the fuck am i doing this?
                    //  it's just a fucking pipe. just
//                    fprintf( file, "%s", buf );
                }
                pclose( ptr );
                return 0;
            }
  //          fclose( file );

//            unlink( filename );

//            return 0;
//        }

        return -1;
    }

    // Resolve symbol name and source location given the path to the executable and an address

    // rename "atos"
    int Addr2Line(char const * const /*program_name*/, void const * const addr, char * buff, size_t buffSize )
    {
        char addr2line_cmd[512] = {0};
        // yeah, just like this. if this doesn't work then we'll try
        //
        pid_t pid = getpid();
        // we must get "fullPath" but we can still make it relative to the program
        // binary, although, maybe we don't (I'd just trim the last 2 or 3 entries TBH
        sprintf(addr2line_cmd, "atos -p%d -fullPath %p", pid,  addr);//program_name, addr );
//        sprintf( addr2line_cmd, "atos -d -o %.256s %p", program_name, addr );
        return Execute( addr2line_cmd, buff, buffSize );
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

    // Print stack trace.
    void PrintStackTrace( )
    {
        int trace_size = 0;
        char ** messages = ( char ** )NULL;

        static const size_t kMaxStackFrames = 64;
        static void * stack_traces[ kMaxStackFrames ];
        trace_size = backtrace( stack_traces, kMaxStackFrames );
        messages = backtrace_symbols( stack_traces, trace_size );

        for ( int i = 0; i < trace_size; ++i )
        {
            //here << messages[i];
            int stackLevel;
            char filename[ 512 ];
            uintptr_t address;
            char symbol[ 512 ];
            uintptr_t symbolOffset;
            uintptr_t functionOffset;
            bool symbolOffsetValid = false;
            bool somethingValid = true;

            // seriously. I expect it's much simple than this
            //  just forward read and use strings wherever possible my love
            // you only want the address from here - that's easy
            //
            if (
                sscanf(
                    messages[ i ],
                    "%d%*[ \t]%s%*[ \t]%" SCNxPTR "%*[ \t]%" SCNxPTR "%*[ \t]+%*[ \t]%" SCNuPTR,
                    &stackLevel,
                    filename,
                    &address,
                    &symbolOffset,
                    &functionOffset) == 5)
            {
                symbolOffsetValid = true;
            }
            // no idea what this is about...
            else if ( sscanf( messages[ i ], "%d%*[ \t]%s%*[ \t]%" SCNxPTR "%*[ \t]%s%*[ \t]+%*[ \t]%" SCNuPTR, &stackLevel, filename, &address, symbol, &functionOffset ) == 5 )
            {
                // doesn't even set SOV.
            }
            else
            {
                somethingValid = false;
            }

            // this can probably be static because, well, it's a crash dump dude. this is the end, my only friend the end.
            const size_t BUFF_SIZE = 4096;
            char buff[ BUFF_SIZE ] = { '\0' };

            if ( somethingValid )
            {
                // what the hell.
                if ( symbolOffsetValid && symbolOffset == 0 )
                {
                    here << "$$$$$$$$$$$$$$$$$";
                    fprintf( stderr, "%3d %-32s   %#16" PRIxPTR "   %#" PRIxPTR " + %" PRIuPTR "\n", stackLevel, filename, address, symbolOffset, functionOffset );
                }
                // don't care
                else if ( FileExists( m_ExeFilename ) && Addr2Line( m_ExeFilename, stack_traces[ i ], buff, BUFF_SIZE) == 0 )
                {
                    char * line_start = nullptr;
                    char * line_end = nullptr;
                    char * file_start = nullptr;
                    char * file_end = nullptr;
                    // dude, you even KNOW how many bytes came back... g'zus
                    char * last = buff + strlen(buff);
                    // scan back until the first ')'
                    while (last >= buff && *last != ')') {
                        last--;
                    }
                    if (last < buff) {
                        // deaded
                        here << "failure";
                    } else {
                        last--;
                        line_end = last;
                        while (last >= buff && *last != ':') {
                            last--;
                        }
                        if (last < buff) {
                            // deaded
                            here << "failure";
                        } else {
                            line_start = last + 1;
                            last--;
                            if (last < buff) {
                                here << "failure";
                            } else {
                                file_end = last;
                                //here << *file_end;
                                while (last >= buff && *last != '(') {
                                    last--;
                                }
                                if (last < buff) {
                                    here << "failure";
                                } else {
                                    file_start = last + 1;
                                    // this, right here, is how you get performance.
//                                    here << "YOU DID IT!";
                                    // i don't want quotes dude.
                                    // the message is jsut 'buff' the first part of which you want anyway
                                    qDebug().nospace().noquote() << "file:" << QLatin1String(file_start, (file_end - file_start) + 1) << ":" << QLatin1String(line_start, 1 + (line_end - line_start)) << ": " << QLatin1String(buff, file_start - 1);//this is my message";
                                    //here << QLatin1String(line_start, line_end - line_start);
                                }
                            }
                        }
                    }

                    //here << buff;
                    // the last bit of buff is (filename:line_number)
                    // SADLY if you have 2 files with the same name I'm not sure
                    // you can disambiguate.

//                    fprintf( stderr, "%3d %-32s   %#16" PRIxPTR "   %s", stackLevel, filename, address, buff );
                }
                else
                {
                    here << "$$$$$$$$$$$$$$$$$";
                    fprintf( stderr, "%3d %-32s   %#16" PRIxPTR "   %#" PRIxPTR " + %" PRIuPTR "\n", stackLevel, filename, address, symbolOffset, functionOffset );
                }
            }
            else
            {
                here << "$$$$$$$$$$$$$$$$$";
                fprintf( stderr, "%s\n", messages[ i ] );
            }
        }
        if (messages)
        {
            free( messages );
        }
    }

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
    bool Initialise( const char * argv )
    {
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
