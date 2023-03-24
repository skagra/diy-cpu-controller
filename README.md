# DIY CPU Controller

This is a micro controller based control unit for the [Diy CPU](https://github.com/skagra/diy-cpu).   

It is a both a stop gap until the control unit proper has been built and is a useful debugging tool for the CPU and its microcode.

This repository contains two projects:

* [controller](controller) - This is the actual control unit code to be uploaded to the micro controller.
* [romtoc](romtoc) - This takes the binary microcode ROMs and decoder ROMs as built as part of the [Diy CPU](https://github.com/skagra/diy-cpu) project and creates C++ code that is compiled into the controller.
