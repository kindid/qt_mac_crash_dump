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

Generally not very interested in anything that fails
The top 2 are a waste of time too (backtrack_symsobl in libdyld.dylib)
=======
# Example
```
file:///../qt_mac_crash_dump/mld/mld.cpp:227:init:
```
Normal program output here until something goes wrong.
```
file:///../qt_mac_crash_dump/mld/mld.cpp:211:sig_handler_generic: Signal caught  SIGSEGV
file:///../qt_mac_crash_dump/mld/mld.cpp:134: mld::dump_stack_trace() (in qt_mac_crash_dump) 
file:///../qt_mac_crash_dump/mld/mld.cpp:216: mld::sig_handler_generic(int, __siginfo*, void*) (in qt_mac_crash_dump) 
file:///../qt_mac_crash_dump/mld/mld.cpp:144:dump_stack_trace: atos_failed:backtrace_symbol: 2   libsystem_platform.dylib            0x00007fff6fb3ff5a _sigtramp + 26
file:///../qt_mac_crash_dump/mld/mld.cpp:144:dump_stack_trace: atos_failed:backtrace_symbol: 3   libsystem_trace.dylib               0x00007fff6fb6c6ee os_log_type_enabled + 425
file:///../../Qt5.11.0/5.11.0/clang_64/lib/QtCore.framework/Headers/qdebug.h:155: QDebug::operator<<(QString const&) (in qt_mac_crash_dump) 
file:///../qt_mac_crash_dump/main.cpp:38: main (in qt_mac_crash_dump) 
file:///../qt_mac_crash_dump/mld/mld.cpp:144:dump_stack_trace: atos_failed:backtrace_symbol: 6   libdyld.dylib                       0x00007fff6f831015 start + 1
file:///../qt_mac_crash_dump/mld/mld.cpp:144:dump_stack_trace: atos_failed:backtrace_symbol: 7   ???                                 0x0000000000000001 0x0 + 1
/Users/kuiash/github/build-qt_mac_crash_dump-Desktop_Qt_5_11_0_clang_64bit-Debug/qt_mac_crash_dump exited with code 1
```
Although not obvious here the file:line prefixes are clickable and will take you straight to the offending lines of code.

You can see several lines of failure in the current code and this saddens me a little! The actual offender is main.cpp:38, well, partly. That's what crashed (null pointer deref) but the cause is, clearly, elsewhere.

# Prologue
Yours,
  Matthew.

