#include <here.h>

#if 0
struct null_out { };
extern null_out _null_out;

// this is the definition of a 'stream'
//  a code block is inserted within it but the operation remains the same
template<typename _t> null_out & operator<<(null_out & _io, _t) { return _io; }
#endif
#if 0
////////////////////////////////////////////////////////////////

// this only needs to exist as a symbol in the system
// normally it'd be a stream object... this just does FA
null_out _null_out;

////////////////////////////////////////////////////////////////

QDebug operator<<(QDebug debug, const std::string & value) {
    debug << QString::fromStdString(value);
    return debug;
}
#endif
// you see... what we are trying to formulate is a
// function (or similar) that takes a variable number
// of parameters (or builds an array) and each of
// those parameter types must support a "to_out"
// function, and those can be hard.
// you don't even know how many bytes long it will
// be but a good guess would be 4K and to always
// allow up to 4k due to the magic of paging.
// this is done by using 3 pages!
//
// one is being output - (the visible part of
// a triple buffer)
// one is the write buffer and the last is
// the overflow.
// therefore there is up to 8K available
// for writing /but/ you are still limited to writing
// 4k. this would fill the main buffer or leave
// just 1 byte free in the overflow in the
// worse case
// if the main buffer is filled then it is swapped out
// and the overflow page is moved to the main page, the
// pages are move (main becomes visible, overflow
// becomes main and any free page, most likely the old
// output buffer) becomes the overflow /and/ the
// pointer is wrapped

// if you need to write data longer than 4K you'll need
// to do it in 4K chunks.
// utility functions help you do this
