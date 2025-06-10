#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "config.h"
#include "paper_cition_api.h"

// 用于接收CURL响应数据的回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// int main() {
//     // 初始化CURL
//     CURL* curl = curl_easy_init();
//     std::string response;
    
//     if (curl) {
//         // 设置URL和回调函数
//         curl_easy_setopt(curl, CURLOPT_URL, config::API_URL.c_str());
//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
//         // 执行请求
//         CURLcode res = curl_easy_perform(curl);
        
//         // 检查是否成功
//         if (res != CURLE_OK) {
//             std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
//         } else {
//             // 解析JSON
//             try {
//                 nlohmann::json json_data = nlohmann::json::parse(response);
//                 std::cout << "sucess get content: " << std::endl;
//                 std::cout << "title: " << json_data["title"] << std::endl;
//                 std::cout << "content: " << json_data["body"] << std::endl;
//                 std::cout << "User ID: " << json_data["userId"] << std::endl;
//                 std::cout << "ID: " << json_data["id"] << std::endl;
//             } catch (const nlohmann::json::parse_error& e) {
//                 std::cerr << "JSON error: " << e.what() << std::endl;
//             }
//         }
        
//         // 清理
//         curl_easy_cleanup(curl);
//     }
    
//     return 0;
// }

// main.cpp
#include "md_converter.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <filesystem>

void printUsage() {
    std::cout << "\n===== Markdown to LaTeX Converter =====\n";
    std::cout << "Available commands:\n";
    std::cout << "  1. convert <input_markdown_file> [output_latex_file]\n";
    std::cout << "     - Convert a markdown file to LaTeX\n";
    std::cout << "     - If output file is not specified, output will be written to input_file_name.tex\n";
    std::cout << "  2. help\n";
    std::cout << "     - Display this help message\n";
    std::cout << "  3. exit\n";
    std::cout << "     - Exit the program\n";
    std::cout << "======================================\n";
}

std::vector<std::string> splitCommand(const std::string& command) {
    std::vector<std::string> args;
    std::string currentArg;
    bool inQuotes = false;

    for (char c : command) {
        if (c == '"') {
            inQuotes = !inQuotes;
        }
        else if (c == ' ' && !inQuotes) {
            if (!currentArg.empty()) {
                args.push_back(currentArg);
                currentArg.clear();
            }
        }
        else {
            currentArg += c;
        }
    }

    if (!currentArg.empty()) {
        args.push_back(currentArg);
    }

    return args;
}

std::string getDefaultOutputFilename(const std::string& inputFile) {
    namespace fs = std::filesystem;

    // Get the file path without extension
    fs::path inputPath(inputFile);
    std::string baseName = inputPath.stem().string();
    std::string directory = inputPath.parent_path().string();

    // Create output path with .tex extension
    fs::path outputPath;
    if (directory.empty()) {
        outputPath = baseName + ".tex";
    }
    else {
        outputPath = directory + "/" + (baseName + ".tex");
    }

    return outputPath.string();
}

bool convertMarkdownToLatex(const std::string& inputFile, std::string outputFile = "") {
    // Read input file
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error: Cannot open input file: " << inputFile << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    // Convert markdown to LaTeX
    MarkdownConverter converter;
    std::string latexContent = converter.convertToLatex(content);

    // If no output file specified, use default name
    if (outputFile.empty()) {
        outputFile = getDefaultOutputFilename(inputFile);
    }

    // Output result to file
    std::ofstream outFile(outputFile);
    if (!outFile) {
        std::cerr << "Error: Cannot open output file: " << outputFile << std::endl;
        return false;
    }

    outFile << latexContent;
    outFile.close();
    std::cout << "Conversion successful. LaTeX content written to " << outputFile << std::endl;

    return true;
}

int main() {
    std::string command;

    // Display initial usage information
    std::cout << "Welcome to Markdown to LaTeX Converter!" << std::endl;
    printUsage();

    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);

        if (command.empty()) {
            continue;
        }

        std::vector<std::string> args = splitCommand(command);

        if (args.empty()) {
            continue;
        }

        if (args[0] == "exit" || args[0] == "quit") {
            std::cout << "Exiting program. Goodbye!" << std::endl;
            break;
        }
        else if (args[0] == "help") {
            printUsage();
        }
        else if (args[0] == "convert") {
            if (args.size() < 2) {
                std::cout << "Error: Missing input file. Usage: convert <input_file> [output_file]" << std::endl;
                continue;
            }

            std::string inputFile = args[1];
            std::string outputFile = (args.size() > 2) ? args[2] : "";

            convertMarkdownToLatex(inputFile, outputFile);
        }
        else {
            std::cout << "Unknown command: " << args[0] << std::endl;
            std::cout << "Type 'help' for available commands." << std::endl;
        }
    }

    return 0;
}