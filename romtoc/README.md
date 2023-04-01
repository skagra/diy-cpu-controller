# ROMTOC

`Romtoc` takes the binary microcode ROMs and decoder ROMs  built as part of the [Diy CPU](https://github.com/skagra/diy-cpu) project or the [Diy CPU Simplified](https://github.com/skagra/diy-cpu-simplified) project and creates equivalent C++ code that is compiled into the controller.

# Execution

`dotnet run <input-file> [output-directory]`

Where

* `input-rom-file` - File containing microcode ROM binary or decoder ROM binary.
* `output-directory` - Directory into which to write *C++* output.  Defaults to `.`

The `Makefile` in the [controller](../controller) project will automatically process the expected ROMs from `../diy-cpu/simplified/ucode/bin` and output all resulting C++ files to `../controller/uc`.




