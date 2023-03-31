# DIY CPU Controller

This is a μcontroller-based control unit for the [Diy CPU](https://github.com/skagra/diy-cpu-meta).   

It is a both a stop gap until the control unit proper has been built and is a useful debugging tool for the CPU and its μcode.

This repository contains two projects:

* [Controller](controller) - This is the actual control unit code to be uploaded to the micro controller.
* [Romtoc](romtoc) - This processes the binary μcode ROMs and decoder ROMs built as part of the [Diy CPU](https://github.com/skagra/diy-cpu) project or the [Diy CPU Simplified](https://github.com/skagra/diy-cpu-simplified) project and creates equivalent C++ code that is compiled into the controller.
