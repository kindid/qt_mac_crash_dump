#ifndef _out_h_
#define _out_h_

#include <QDebug>
#include <string>
#if 0
// TODO:rename this file _dbg.h or maybe just dbg.h
// or print 'file'. i don't fucking care
#define _dbg qDebug() << __PRETTY_FUNCTION__ << '@' << __LINE__
#define here qDebug() << __PRETTY_FUNCTION__ << '@' << __LINE__
#endif

#define _MARK __FILE__ ":" __LINE__

// will the space get stripped?
#define _dbg (qDebug().nospace().noquote() << "file:///" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ":").space()
#define here (qDebug().nospace().noquote() << "file:///" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ":").space()

// own debug... it really is an object called _dbg
// ideally there's one per thread
//  anything that has a std::string str(blah) function
//  is usable. maybe we should pass in a string
//  or stream, that is what happens in other parts of the system e.g.
//  ostream etc

//QDebug operator<<(QDebug debug, const std::string & value);

struct null_out { };
extern null_out _null_out;

// this is the definition of a 'stream'
//  a code block is inserted within it but the operation remains the same

// a 'stream' is an object - that much is obvious
//  obvious because you're bound to need a few variables - possibly many

// this is tricky - dead code ellimination SHOULD get rid of this
// BUT - if you output the result of an expression you may find that
// the expression is executed anyway - this should NOT be the case
//  truth is that stuff printed should be a simple variable (and immutable)
//  OR a constant (immediate) value
template<typename _t> null_out & operator<<(null_out & _io, _t) { return _io; }

//#ifdef dbg_heavy
//    #define heavy here
//    #undef dbg_heavy
//#else
    #define heavy _null_out
//#endif

void unwind_stack();

#endif
