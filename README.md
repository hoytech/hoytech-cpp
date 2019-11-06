# Hoytech C++ Libs

These libraries must be compiled with C++11 or higher. They are licensed under the 2-clause BSD license so you can basically do whatever you want with them.

Documentation is very sparse for now. See the header files for details.

## protected_queue

Thread-safe queue implementation. Header-only library.

## timer

Spawns a thread which will run timers. Timers are cancellable. See the file `ex/timer_test.cpp` for a usage example.

## assert_zerocopy

Assertions to test whether memory is shared (or not).

## error

Simple base class for run-time exceptions, and a way to construct them.

## hex

Convert strings to/from hexadecimal encoding.
