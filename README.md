A simple C++ utility to **split** a large file into two parts (optionally to different drives) and **merge** them back together. Useful for moving large files between drives with limited space.

## Features

- **Split** a file into two parts, saving each part to a specified drive or directory.
- **Merge** two parts back into a single file.
- Progress display for both splitting and merging.
- Command-line interface with flexible options.

## Requirements

- C++17 compiler
- cxxopts (included header-only)

## Building

Use your preferred C++ compiler. Example with g++:

```sh
g++ splitter.cpp -I./include -o splitter
```

## Usage

### Split a File

```sh
splitter.exe -s --mainfile=<file> --drive1=<drive> --drive2=<drive> --p1size=<size_in_gb>
```

- `--mainfile` : Path to the file you want to split.
- `--drive1`   : Drive letter or directory for the first part (e.g., `D`).
- `--drive2`   : (Optional) Drive letter or directory for the second part. If omitted, part2 is saved in the current directory.
- `--p1size`   : Size of the first part in GB.

**Example:**

```sh
splitter.exe -s --mainfile=bigfile.bin --drive1=D --drive2=E --p1size=3.5
```

### Merge Two Parts

```sh
splitter.exe -m --file1=<part1_path> --file2=<part2_path>
```

- `--file1` : Path to the first part (output file, will be appended to).
- `--file2` : Path to the second part (input file).

**Example:**

```sh
splitter.exe -m --file1=D:/part1.bin --file2=E:/part2.bin
```

### Help

```sh
splitter.exe --help
```

## Notes

- The program creates files named `part1.bin` and `part2.bin` in the specified directories.
- Make sure you have enough free space on the target drives.
- The merge operation appends `part2.bin` to `part1.bin`.

## License

See cxxopts.hpp for third-party license.

---

**Author:**  
Kaleab Nigusse

**Contributions and issues are welcome!**
