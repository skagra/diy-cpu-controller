public static class RomToC
{
    private const int _valuesPerRow = 16;

    public static void Usage()
    {
        Console.WriteLine("dotnet run <input-rom-file> [output-directory]");
    }

    private static string SanitizeFilename(string filename)
    {
        return filename.Replace('-', '_');
    }

    private static string FilenameToArrayName(string filename)
    {
        return filename;
    }

    public static int Main(string[] args)
    {
        if (args.Length == 0 || args.Length > 2)
        {
            Usage();
            return -1;
        }

        var inputFilePath = args[0];
        var outputDir = (args.Length == 2) ? args[1] : "";
        var outputFilename = SanitizeFilename(Path.GetFileNameWithoutExtension(inputFilePath));
        var outputFilePath = Path.Join(outputDir, $"{outputFilename}.h");
        var arrayName = FilenameToArrayName(outputFilename);
        var armour = $"_{arrayName.ToUpper()}_DOT_H_";

        Console.WriteLine($"Converting ROM file '{inputFilePath}'");
        Console.WriteLine($"Output will be written to '{outputFilePath}'");
        Console.WriteLine($"Array name '{arrayName}'");

        var fileBytes = File.ReadAllBytes(inputFilePath);
        var outputWriter = new StreamWriter(outputFilePath);

        outputWriter.WriteLine("// This file was auto-generated - DO NOT EDIT");
        outputWriter.WriteLine($"// Source file: '{inputFilePath}'");
        outputWriter.WriteLine($"// Date: {DateTime.UtcNow}");
        outputWriter.WriteLine();
        outputWriter.WriteLine($"#ifndef {armour}");
        outputWriter.WriteLine($"#define {armour}");
        outputWriter.WriteLine();
        outputWriter.WriteLine("#include <avr/pgmspace.h>");
        outputWriter.WriteLine();
        outputWriter.WriteLine($"const byte {arrayName}[] PROGMEM {{");
        outputWriter.Write("   ");

        for (var index = 0; index < fileBytes.Length; index++)
        {
            outputWriter.Write($"0x{fileBytes[index]:X2}");
            if (index != fileBytes.Length - 1)
            {
                outputWriter.Write(", ");
                if (index % _valuesPerRow == _valuesPerRow - 1)
                {
                    outputWriter.WriteLine();
                    outputWriter.Write("   ");
                }
            }
        }

        outputWriter.WriteLine("\n};");
        outputWriter.WriteLine();
        outputWriter.WriteLine("#endif");
        outputWriter.Close();

        return 0;
    }
}