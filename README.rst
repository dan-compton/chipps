
Chipps
======

Chips is a C++ implementation of the Chip-8 interpreter.


Dependencies
------------

* make
* gcc
* ncurses

Compilation
-----------

Typical makefile setup...

.. code-block:: bash
    make
    ./c8 "filename.ch8"


TODO and Known Issues
---------------------
This implementation is a work in progress.
Currently, the implementation is broken due to
bug(s) in opcode execution. A debug mode was added
to provide a way to step through execution and view
disassembled ops.

TODO
^^^^
* Sound
* Key Presses
* Font test


