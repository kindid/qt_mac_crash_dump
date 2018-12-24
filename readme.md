branch(2018_12_23_12_35)
=======
## Qt Mac Crash Dump

# Technologies: macOS, OSX, Qt, Crash, Debug

Sometimes I'm very lazy. When my programs crash, as they innevitably do, I can't even bo bothered to fire up the debugger.

So, what happens is I include the "Mouldy" library. It self initiaises and captures a whole bunch of signals like seg fault, divide by 0 etc and gives you a stack dump in the "Application Output" frame of Qt Creator.

The error messages are formatted according to "here.h" which prefixes a file and line number so you can click straight back to where the error came from. Basically, it walks the stack.

It does this while also popen'ing "atos" which converts binary addresses in the current application, as determined by getpid(), back into file/line number pairings.

It seems like profiling is no big challenge except that you won't be able to call atos while profiling, because that's insane. Instead bundle up the call stack traces until the end of execution and clear up then.

If you get output from any of the "mld" functions then it's because something isn't knowable or parsable. Maybe we can improve on this.

Yours,
  Matthew.

