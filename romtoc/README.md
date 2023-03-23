# ROMTOC

`Romtoc` takes the binary microcode ROMs and decoder ROMs  built as part of the [Diy CPU](https://github.com/skagra/diy-cpu) project and equivalent creates C++ code that is compiled into the controller.

# Execution

`dotnet run <input-file> [output-directory]`

Where

* `input-rom-file` - File containing microcode ROM or decoder ROM.
* `output-directory` - Directory into which to write *C++* output.  Defaults to `.`

`Makefile` will automatically process the expected ROMs from `../../diy-cpu/simplified/ucode/bin/` and output all resulting files to `../uc`.




