#include <string>
#include <iostream>
#include <fstream>
#include "cxxopts.hpp"
#include <iomanip> // for std::setw and std::fixed

unsigned long long getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return 0;
    return static_cast<unsigned long long>(file.tellg());
}

void appendFileWithProgress(const std::string& partfile1Path, const std::string& partfile2Path) {
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

int main(int argc, char* argv[])
{
    cxxopts::Options options("merge", "Merge two binary files with progress display.\n"
        "--file1 <file> --file2 <file>");

    options.add_options()
        ("f1,file1", "First part file (output)", cxxopts::value<std::string>())
        ("f2,file2", "Second part file (input)", cxxopts::value<std::string>())
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help") || !result.count("file1") || !result.count("file2")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::string partfile1 = result["file1"].as<std::string>();
    std::string partfile2 = result["file2"].as<std::string>();

    appendFileWithProgress(partfile1, partfile2);
    return 0;
}
