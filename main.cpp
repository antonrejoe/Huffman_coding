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

CompressionMetrics analyzeCompression(const std::string &path)
{
    CompressionMetrics metrics;

    // Original File Size
    std::filesystem::path originalFile(path);
    metrics.originalSize = std::filesystem::file_size(originalFile);

    Huffman huffman(path);

    // Read file content for probability calculation
    std::ifstream inputFile(path);
    std::string text((std::istreambuf_iterator<char>(inputFile)),
                     std::istreambuf_iterator<char>());

    // Calculate symbol probabilities
    metrics.symbolProbabilities = huffman.calculateSymbolProbabilities(text);

    auto start = std::chrono::high_resolution_clock::now();

    std::string outputPath = huffman.compress();

    auto end = std::chrono::high_resolution_clock::now();

    // Compressed File Size
    std::filesystem::path compressedFile(outputPath);
    metrics.compressedSize = std::filesystem::file_size(compressedFile);

    // Compute Metrics
    metrics.compressionRatio = 1.0 - (static_cast<double>(metrics.compressedSize) / metrics.originalSize);
    metrics.timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;

    return metrics;
}

void exportMetricsToCSV(const std::vector<CompressionMetrics> &metrics)
{
    std::ofstream csvFile("compression_metrics.csv");
    csvFile << "Filename,Original Size,Compressed Size,Compression Ratio,Time (s),Symbol Probabilities\n";

    for (const auto &metric : metrics)
    {
        // Convert probabilities to a string
        std::string probString;
        for (const auto &[ch, prob] : metric.symbolProbabilities)
        {
            // Handle special characters
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
}

void printDetailedMetrics(const std::vector<CompressionMetrics> &metrics)
{
    for (const auto &metric : metrics)
    {
        std::cout << "File: " << metric.filename << "\n";
        std::cout << "Original Size: " << metric.originalSize << " bytes\n";
        std::cout << "Compressed Size: " << metric.compressedSize << " bytes\n";
        std::cout << "Compression Ratio: " << metric.compressionRatio * 100 << "%\n";
        std::cout << "Compression Time: " << metric.timeTaken << " seconds\n";

        std::cout << "Symbol Probabilities:\n";
        for (const auto &[ch, prob] : metric.symbolProbabilities)
        {
            // Handle special characters
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
            std::cout << charRepr << ": " << prob * 100 << "%\n";
        }
        std::cout << "\n";
    }
}

int main()
{
    std::vector<std::string> testFiles = {
        "sample/sample1.txt", "sample/sample2.txt", "sample/sample3.txt"};

    std::vector<CompressionMetrics> metrics;

    for (const auto &file : testFiles)
    {
        CompressionMetrics metric = analyzeCompression(file);
        metric.filename = file;
        metrics.push_back(metric);
    }

    // Print detailed metrics
    printDetailedMetrics(metrics);

    // Export to CSV
    exportMetricsToCSV(metrics);

    return 0;
}
