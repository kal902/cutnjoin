#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "cxxopts.hpp"

using namespace std;

unsigned long long getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return 0;
    return static_cast<unsigned long long>(file.tellg());
}

void splitFile(const string &inputPath, const unsigned long long &part1Size, const string &outputDir1, const string &outputDir2)
{
    std::cout << "split operation started" << endl;

    ifstream inFile(inputPath, ios::binary);
    if (!inFile)
    {
        cerr << "Failed to open input file.\n";
        return;
    }

    string part1Path = outputDir1 + "part1.bin";
    string part2Path = outputDir2 + "part2.bin";

    ofstream out1(part1Path, ios::binary);
    ofstream out2(part2Path, ios::binary);

    if (!out1 || !out2)
    {
        cerr << "Failed to open output files.\n";
        return;
    }

    const size_t bufferSize = 1024 * 1024;
    char buffer[bufferSize];
    size_t bytesWritten = 0;
    int last_progress = -5;

    // Write first part
    while (bytesWritten < part1Size && inFile)
    {
        size_t toRead = static_cast<size_t>(
            std::min(static_cast<unsigned long long>(bufferSize), part1Size - bytesWritten));

        inFile.read(buffer, toRead);
        size_t bytesRead = inFile.gcount();

        out1.write(buffer, bytesRead);
        bytesWritten += bytesRead;

        int current_progress = static_cast<int>((static_cast<double>(bytesWritten) / part1Size) * 100.0);
        if (current_progress >= last_progress + 5) {
            std::cout << "\rProgress 1/2: " << current_progress << "%";
            std::cout.flush();
            last_progress = current_progress;
        }
    }
    out1.close();

    // Write second part (rest of the file)
    bytesWritten = 0;
    last_progress = -5;
    unsigned long long totalSize = getFileSize(inputPath);
    unsigned long long remaining_part_size = totalSize - part1Size;

    while (inFile.read(buffer, bufferSize))
    {
        out2.write(buffer, bufferSize);
        bytesWritten += bufferSize;

        int current_progress = static_cast<int>((static_cast<double>(bytesWritten) / remaining_part_size) * 100.0);
        if (current_progress >= last_progress + 5) {
            std::cout << "\rProgress 2/2: " << current_progress << "%";
            std::cout.flush();
            last_progress = current_progress;
        }
    }
    if (inFile.gcount() > 0)
    {
        out2.write(buffer, inFile.gcount());
    }
    std::cout << "\rProgress 2/2: 100%";
    std::cout.flush();

    out2.close();
    inFile.close();

    std::cout << "\nFile split successfully:\n"
              << "- " << part1Path << "\n"
              << "- " << part2Path << endl;
}

void merge(const std::string& partfile1Path, const std::string& partfile2Path) {
    const std::size_t bufferSize = 1 * 1024 * 1024;
    char buffer[bufferSize];

    std::ofstream outFile(partfile1Path, std::ios::binary | std::ios::app);
    if (!outFile) {
        std::cerr << "Failed to open " << partfile1Path << " for appending.\n";
        return;
    }

    std::ifstream inFile(partfile2Path, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open " << partfile2Path << " for reading.\n";
        return;
    }

    unsigned long long totalSize = getFileSize(partfile2Path);
    unsigned long long bytesCopied = 0;
    int last_progress = -5;

    while (inFile) {
        inFile.read(buffer, bufferSize);
        std::streamsize bytesRead = inFile.gcount();

        if (bytesRead > 0) {
            outFile.write(buffer, bytesRead);
            bytesCopied += bytesRead;

            int current_progress = static_cast<int>((static_cast<double>(bytesCopied) / totalSize) * 100.0);
            if (current_progress >= last_progress + 5) {
                std::cout << "\rMerging: " << current_progress << "%";
                std::cout.flush();
                last_progress = current_progress;
            }
        }
    }
    std::cout << "\rMerging: 100%" << std::endl;
    std::cout << "Appended " << partfile2Path << " to " << partfile1Path << " successfully.\n";
}

int main(int argc, char *argv[])
{
    cxxopts::Options options("splitter", "Split or merge files.\n"
        "Split: splitter.exe -s --mainfile=<file> --drive1=<drive> --drive2=<drive> --p1size=<size_in_gb>\n"
        "Merge: splitter.exe -m --file1=<file> --file2=<file>");

    options.add_options()
        ("s,split", "Split mode")
        ("m,merge", "Merge mode")
        ("mainfile", "Main Input file", cxxopts::value<std::string>())
        ("drive1", "Drive one", cxxopts::value<std::string>())
        ("drive2", "Drive two", cxxopts::value<std::string>()->default_value(""))
        ("p1size", "Part one size in GB", cxxopts::value<double>())
        ("file1", "First part file (output)", cxxopts::value<std::string>())
        ("file2", "Second part file (input)", cxxopts::value<std::string>())
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help") || argc == 1) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("split")) {
        // Split mode
        if (!result.count("mainfile") || !result.count("drive1") || !result.count("p1size")) {
            std::cerr << "Missing required arguments for split mode.\n";
            std::cout << options.help() << std::endl;
            return 1;
        }
        std::string inputfile = result["mainfile"].as<std::string>();
        std::string outdir1 = result["drive1"].as<std::string>() + ":/";
        std::string outdir2;
        if (result.count("drive2") && !result["drive2"].as<std::string>().empty()) {
            outdir2 = result["drive2"].as<std::string>() + ":/";
        } else {
            outdir2 = "";
            std::cout << "--drive2 not provided, so part2 will be saved to current folder" << endl;
        }
        double part1size_in_gb = result["p1size"].as<double>();
        unsigned long long size_in_bytes = static_cast<unsigned long long>(part1size_in_gb * 1024 * 1024 * 1024);
        splitFile(inputfile, size_in_bytes, outdir1, outdir2);
    }
    else if (result.count("merge")) {
        // Merge mode
        if (!result.count("file1") || !result.count("file2")) {
            std::cerr << "Missing required arguments for merge mode.\n";
            std::cout << options.help() << std::endl;
            return 1;
        }
        std::string partfile1 = result["file1"].as<std::string>();
        std::string partfile2 = result["file2"].as<std::string>();
        merge(partfile1, partfile2);
    }
    else {
        std::cerr << "You must specify either -s (split) or -m (merge) mode.\n";
        std::cout << options.help() << std::endl;
        return 1;
    }
    return 0;
}