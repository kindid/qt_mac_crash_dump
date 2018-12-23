///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef MLD2_H
#define MLD2_H

#include <stdlib.h>

namespace ExceptionHandler
{
    //char m_ExeFilename[ 512 ];
// singluar public entry point. interesting.
    bool Initialise( const char * argv );


    int Execute( const char * cmd, char * buf, size_t bufSize );
    int Addr2Line(char const * const program_name, void const * const addr, char * buff, size_t buffSize );
    bool FileExists( const char * filename );
    void PrintStackTrace( );
    void Handler( int sig, siginfo_t * siginfo, void * context );
}

#endif // MLD2_H
