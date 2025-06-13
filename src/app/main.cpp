#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "md_converter.h"

void printUsage()
{
    std::cout << "\n===== Markdown to LaTeX Converter =====\n";
    std::cout << "Available commands:\n";
    std::cout << "  1. convert <input_markdown_file> [output_latex_file]\n";
    std::cout << "     - Convert a markdown file to LaTeX\n";
    std::cout << "     - If output file is not specified, output will be written to "
                 "input_file_name.tex\n";
    std::cout << "  2. help\n";
    std::cout << "     - Display this help message\n";
    std::cout << "  3. exit\n";
    std::cout << "     - Exit the program\n";
    std::cout << "======================================\n";
}

std::vector<std::string> splitCommand(const std::string &command)
{
    std::vector<std::string> args;
    std::string currentArg;
    bool inQuotes = false;

    for (char chr : command) // renamed from 'c' to 'chr'
    {
        if (chr == '"')
        {
            inQuotes = !inQuotes;
        }
        else if (chr == ' ' && !inQuotes)
        {
            if (!currentArg.empty())
            {
                args.push_back(currentArg);
                currentArg.clear();
            }
        }
        else
        {
            currentArg += chr;
        }
    }

    if (!currentArg.empty())
    {
        args.push_back(currentArg);
    }

    return args;
}

std::string getDefaultOutputFilename(const std::string &inputFile)
{
    namespace fs = std::filesystem;

    // Get the file path without extension
    fs::path inputPath(inputFile);
    std::string baseName = inputPath.stem().string();
    std::string directory = inputPath.parent_path().string();

    // Create output path with .tex extension
    fs::path outputPath;
    if (directory.empty())
    {
        outputPath = baseName + ".tex";
    }
    else
    {
        outputPath = directory + "/" + (baseName + ".tex");
    }

    return outputPath.string();
}

bool convertMarkdownToLatex(const std::string &inputFile, std::string outputFile = "")
{
    // Read input file
    std::ifstream inFile(inputFile);
    if (!inFile)
    {
        std::cerr << "Error: Cannot open input file: " << inputFile << "\n";
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    // Convert markdown to LaTeX
    MarkdownConverter converter;
    std::string latexContent = converter.convertToLatex(content);

    // If no output file specified, use default name
    if (outputFile.empty())
    {
        outputFile = getDefaultOutputFilename(inputFile);
    }

    // Output result to file
    std::ofstream outFile(outputFile);
    if (!outFile)
    {
        std::cerr << "Error: Cannot open output file: " << outputFile << "\n";
        return false;
    }

    outFile << latexContent;
    outFile.close();
    std::cout << "Conversion successful. LaTeX content written to " << outputFile << "\n";

    return true;
}

int main()
{
    std::string command;

    // Display initial usage information
    std::cout << "Welcome to Markdown to LaTeX Converter!\n";
    printUsage();

    while (true)
    {
        std::cout << "\n> ";
        std::getline(std::cin, command);

        if (command.empty())
        {
            continue;
        }

        std::vector<std::string> args = splitCommand(command);

        if (args.empty())
        {
            continue;
        }

        if (args[0] == "exit" || args[0] == "quit")
        {
            std::cout << "Exiting program. Goodbye!\n";
            break;
        }

        if (args[0] == "help")
        {
            printUsage();
        }
        else if (args[0] == "convert")
        {
            if (args.size() < 2)
            {
                std::cout
                    << "Error: Missing input file. Usage: convert <input_file> [output_file]\n";
                continue;
            }

            const std::string &inputFile = args[1];
            std::string outputFile = (args.size() > 2) ? args[2] : "";

            convertMarkdownToLatex(inputFile, outputFile);
        }
        else
        {
            std::cout << "Unknown command: " << args[0] << "\n";
            std::cout << "Type 'help' for available commands.\n";
        }
    }

    return 0;
}