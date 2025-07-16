#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "cxxopts.hpp"
#include <iomanip> // for std::setw and std::fixed

using namespace std;

unsigned long long getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);  // open and seek to end
    if (!file) return 0;

    return static_cast<unsigned long long>(file.tellg());  // get position (i.e., size)
}

void splitFile(const string &inputPath, const unsigned long long &part1Size, const string &outputDir1, const string &outputDir2)
{
    std::cout << "split operation started"<<endl;

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

    //const size_t bufferSize = 4 * 1024 * 1024;// 10 MB
    const size_t bufferSize = 1024 * 1024;// 1 MB
    char buffer[bufferSize];
    size_t bytesWritten = 0;
    int last_progress = -5;

    // Write first part
    while (bytesWritten < part1Size && inFile)
    {
        // size_t toRead = min(bufferSize, part1Size - bytesWritten);
        // size_t toRead = std::min(bufferSize, static_cast<size_t>(part1Size - bytesWritten));
        size_t toRead = static_cast<size_t>(
            std::min(static_cast<unsigned long long>(bufferSize), part1Size - bytesWritten));

        inFile.read(buffer, toRead);
        size_t bytesRead = inFile.gcount();

        out1.write(buffer, bytesRead);
        bytesWritten += bytesRead;
        
        //display progress
        int current_progress = static_cast<int>( (static_cast<double>(bytesWritten) / part1Size) * 100.0);
        if(current_progress >= last_progress + 5){
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
    unsigned long long remaning_part_size = totalSize - part1Size;

    while (inFile.read(buffer, bufferSize))
    {
        out2.write(buffer, bufferSize);
        bytesWritten += bufferSize;

        //display progress every 5%
        int current_progress = static_cast<int>((static_cast<double>(bytesWritten) / remaning_part_size) * 100.0);
        if(current_progress >= last_progress +5){
            std::cout << "\rProgress 2/2: " << current_progress << "%";
            std::cout.flush();
            last_progress = current_progress;
        }
        
    }
    // Handle any leftover bytes
    if (inFile.gcount() > 0)
    {
        out2.write(buffer, inFile.gcount());
    }
    std::cout << "\rProgress 2/2: " << "100%";
    std::cout.flush();

    out2.close();
    inFile.close();


    std::cout << "File split successfully:\n"
         << "- " << part1Path << "\n"
         << "- " << part2Path << endl;

}


int main(int argc, char *argv[])
{
    // --mainfile , --driveone, --drivetwo, --part1size
    cxxopts::Options options("split", "exampe using split: note 'split' only splits a file in two parts\n--mainfile <file> --part1size <size_in_gb> --driveone <drive1> --drivetwo <drive2>\n drive one is used to store part1 and drive two is used to store part2\n just pass the drive letter when passing drives");

    options.add_options()
    ("f,mainfile", "Main Input file", cxxopts::value<std::string>())
    ("d1,drive1", "drive one", cxxopts::value<std::string>())
    ("d2,drive2", "drive two", cxxopts::value<std::string>())
    ("sp1,p1size", "part one size in GB", cxxopts::value<double>())
    ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }

    double part1size_in_gb;
    string inputfile, outdir1, outdir2;

    if (result.count("mainfile"))
    {
        inputfile = result["mainfile"].as<std::string>();
    }
    else
    {
        std::cerr << "You must provide a --driveone argument.\n";
    }

    if (result.count("p1size"))
    {
        part1size_in_gb = result["p1size"].as<double>();
    }
    else
    {
        std::cerr << "You must provide a --p1size argument.\n";
    }

    if (result.count("drive1"))
    {
        outdir1 = result["drive1"].as<std::string>() + ":/";
    }
    else
    {
        std::cerr << "You must provide a --drive1 argument.\n";
    }

    if (result.count("drive2"))
    {
        outdir2 = result["drive2"].as<std::string>() + ":/";
    }
    else
    {
        outdir2 = ""; // not provided(no second drive inserted), so just put it inside the current directory
        std::cout << "--drive2 not provided, so part2 will be saved to current folder" << endl;
    }

    unsigned long long size_in_bytes = static_cast<unsigned long long>(part1size_in_gb * 1024 * 1024 * 1024);

    splitFile(inputfile, size_in_bytes, outdir1, outdir2);
    return 0;
}
