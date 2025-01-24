#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include "Huffman_coding.h"

struct CompressionMetrics
{
    std::string filename;
    long originalSize;
    long compressedSize;
    double compressionRatio;
    double timeTaken;
    std::unordered_map<char, double> symbolProbabilities;
};

// Function to analyze compression and collect metrics
CompressionMetrics analyzeCompression(const std::string &path)
{
    CompressionMetrics metrics;

    std::filesystem::path originalFile(path);
    metrics.filename = originalFile.filename().string();
    metrics.originalSize = std::filesystem::file_size(originalFile);

    Huffman huffman(path);

    std::ifstream inputFile(path);
    std::string text((std::istreambuf_iterator<char>(inputFile)),
                     std::istreambuf_iterator<char>());

    metrics.symbolProbabilities = huffman.calculateSymbolProbabilities(text);

    auto start = std::chrono::high_resolution_clock::now();
    std::string outputPath = huffman.compress();
    auto end = std::chrono::high_resolution_clock::now();

    std::filesystem::path compressedFile(outputPath);
    metrics.compressedSize = std::filesystem::file_size(compressedFile);

    metrics.compressionRatio = 1.0 - (static_cast<double>(metrics.compressedSize) / metrics.originalSize);
    metrics.timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;

    return metrics;
}

// Function to export metrics to a CSV file
void exportMetricsToCSV(const std::vector<CompressionMetrics> &metrics)
{
    std::ofstream csvFile("compression_metrics.csv");
    csvFile << "Filename,Original Size,Compressed Size,Compression Ratio (%),Time (s),Symbol Probabilities\n";

    for (const auto &metric : metrics)
    {
        std::string probString;
        for (const auto &[ch, prob] : metric.symbolProbabilities)
        {
            std::string charRepr;
            switch (ch)
            {
            case '\n':
                charRepr = "\\n";
                break;
            case '\t':
                charRepr = "\\t";
                break;
            case ' ':
                charRepr = "SPACE";
                break;
            default:
                charRepr = std::string(1, ch);
            }
            probString += charRepr + ":" + std::to_string(prob * 100) + "%; ";
        }

        csvFile << metric.filename << ","
                << metric.originalSize << ","
                << metric.compressedSize << ","
                << metric.compressionRatio * 100.0 << ","
                << metric.timeTaken << ","
                << "\"" << probString << "\"\n";
    }
    csvFile.close();
    std::cout << "\nMetrics exported to compression_metrics.csv\n";
}
void printDetailedMetrics(const std::vector<CompressionMetrics> &metrics)
{
    for (const auto &metric : metrics)
    {
        std::cout << "Metrics for file: " << metric.filename << "\n";
        std::cout << "Symbol Probabilities:\n";

        for (const auto &[ch, prob] : metric.symbolProbabilities)
        {
            std::string charRepr;
            switch (ch)
            {
            // Whitespace characters
            case '\n':
                charRepr = "\\n (Newline)";
                break;
            case '\t':
                charRepr = "\\t (Tab)";
                break;
            case '\r':
                charRepr = "\\r (Carriage Return)";
                break;
            case '\f':
                charRepr = "\\f (Form Feed)";
                break;
            case '\v':
                charRepr = "\\v (Vertical Tab)";
                break;
            case ' ':
                charRepr = "SPACE";
                break;

            // Control characters
            case '\b':
                charRepr = "\\b (Backspace)";
                break;
            case '\a':
                charRepr = "\\a (Bell/Alert)";
                break;
            case '\0':
                charRepr = "\\0 (Null)";
                break;
            case '\x1B':
                charRepr = "ESC (Escape)";
                break;

            // Punctuation and special symbols
            case '\\':
                charRepr = "\\\\ (Backslash)";
                break;
            case '\'':
                charRepr = "\\' (Single Quote)";
                break;
            case '\"':
                charRepr = "\\\" (Double Quote)";
                break;
            case '`':
                charRepr = "` (Backtick)";
                break;

            // Brackets and braces
            case '(':
                charRepr = "( (Open Parenthesis)";
                break;
            case ')':
                charRepr = ") (Close Parenthesis)";
                break;
            case '[':
                charRepr = "[ (Open Square Bracket)";
                break;
            case ']':
                charRepr = "] (Close Square Bracket)";
                break;
            case '{':
                charRepr = "{ (Open Curly Brace)";
                break;
            case '}':
                charRepr = "} (Close Curly Brace)";
                break;

            // Mathematical and logical symbols
            case '+':
                charRepr = "+ (Plus)";
                break;
            case '-':
                charRepr = "- (Minus)";
                break;
            case '*':
                charRepr = "* (Asterisk)";
                break;
            case '/':
                charRepr = "/ (Forward Slash)";
                break;
            case '%':
                charRepr = "% (Percent)";
                break;
            case '=':
                charRepr = "= (Equals)";
                break;
            case '<':
                charRepr = "< (Less Than)";
                break;
            case '>':
                charRepr = "> (Greater Than)";
                break;
            case '&':
                charRepr = "& (Ampersand)";
                break;
            case '|':
                charRepr = "| (Vertical Bar)";
                break;
            case '^':
                charRepr = "^ (Caret)";
                break;
            case '~':
                charRepr = "~ (Tilde)";
                break;

            // Punctuation
            case '.':
                charRepr = ". (Period/Dot)";
                break;
            case ',':
                charRepr = ", (Comma)";
                break;
            case ':':
                charRepr = ": (Colon)";
                break;
            case ';':
                charRepr = "; (Semicolon)";
                break;
            case '!':
                charRepr = "! (Exclamation)";
                break;
            case '?':
                charRepr = "? (Question Mark)";
                break;

            default:
                if (std::iscntrl(ch))
                {
                    // For other control characters, show hex representation
                    std::stringstream ss;
                    ss << "\\x" << std::hex << static_cast<int>(static_cast<unsigned char>(ch))
                       << " (Control Character)";
                    charRepr = ss.str();
                }
                else
                {
                    charRepr = std::string(1, ch);
                }
            }

            // Print with percentage, rounded to 2 decimal places
            printf("  '%-20s': %.2f%%\n", charRepr.c_str(), prob * 100);
        }
        std::cout << "\n"; // Separate metrics for different files
    }
}

int main()
{
    std::vector<CompressionMetrics> metricsList;

    std::string folderPath = "sample"; // Folder containing input text files

    // Check if the input folder exists
    if (!std::filesystem::exists(folderPath))
    {
        std::cout << "Input folder '" << folderPath << "' does not exist.\n";
        return 1;
    }

    // Analyze each text file in the input folder
    for (const auto &entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::cout << "Compressing: " << filePath << std::endl;

            CompressionMetrics metrics = analyzeCompression(filePath);
            metricsList.push_back(metrics);
        }
    }

    // Print detailed metrics to the console
    // printDetailedMetrics(metricsList);

    // Export all metrics to CSV
    exportMetricsToCSV(metricsList);

    std::cout << "\nCompression analysis completed.\n";
    return 0;
}

// g++ -I gnuplot-iostream/ main.cpp -o main -lboost_iostreams -lboost_system -lboost_filesystem && ./main