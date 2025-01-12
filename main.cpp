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

// Function to print detailed metrics to the console
void printDetailedMetrics(const std::vector<CompressionMetrics> &metrics)
{
    for (const auto &metric : metrics)
    {
        std::cout << "\nFile: " << metric.filename << "\n";
        std::cout << "Original Size: " << metric.originalSize << " bytes\n";
        std::cout << "Compressed Size: " << metric.compressedSize << " bytes\n";
        std::cout << "Compression Ratio: " << metric.compressionRatio * 100.0 << "%\n";
        std::cout << "Time Taken: " << metric.timeTaken << " seconds\n";
        std::cout << "Symbol Probabilities:\n";

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
            std::cout << "  '" << charRepr << "': " << prob * 100 << "%\n";
        }
        std::cout << "------------------------------------------\n";
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
