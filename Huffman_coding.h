#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <filesystem>
#include <fstream>
#include <bitset>
#include <stdexcept>
using namespace std;

class Huffman
{
public:
    class HeapNode
    {
    public:
        char character;
        int frequency;
        unique_ptr<HeapNode> left;
        unique_ptr<HeapNode> right;

        HeapNode(char ch, int freq) : character(ch),
                                      frequency(freq) {}

        struct compare
        {
            bool operator()(const HeapNode *left, const HeapNode *right) const
            {
                return left->frequency > right->frequency;
            }
        };
    };

private:
    string path;
    priority_queue<
        HeapNode *,
        vector<HeapNode *>,
        HeapNode::compare>
        heap;
    unordered_map<char, string> codes;
    unordered_map<string, char> reverse_mapping;

    // Memory cleanup method
    void clearHeap()
    {
        while (!heap.empty())
        {
            delete heap.top();
            heap.pop();
        }
    }

    unordered_map<char, int> makeFrequencyDict(const string &text)
    {
        unordered_map<char, int> frequency;
        for (char ch : text)
        {
            frequency[ch]++;
        }
        return frequency;
    }

    void makeHeap(const unordered_map<char, int> &frequency)
    {
        for (const auto &[ch, freq] : frequency)
        {
            heap.push(new HeapNode(ch, freq));
        }
    }

    unique_ptr<HeapNode> mergeNodes()
    {
        while (heap.size() > 1)
        {
            HeapNode *node1 = heap.top();
            heap.pop();
            HeapNode *node2 = heap.top();
            heap.pop();

            auto merged = make_unique<HeapNode>('\0', node1->frequency + node2->frequency);
            merged->left.reset(node1);
            merged->right.reset(node2);

            heap.push(merged.release());
        }

        HeapNode *root = heap.top();
        heap.pop();
        return unique_ptr<HeapNode>(root);
    }

    void makeCodesHelper(HeapNode *root, const string &currentCode)
    {
        if (!root)
            return;

        if (root->character != '\0')
        {
            codes[root->character] = currentCode;
            reverse_mapping[currentCode] = root->character;
            return;
        }

        makeCodesHelper(root->left.get(), currentCode + "0");
        makeCodesHelper(root->right.get(), currentCode + "1");
    }

    void makeCodes(HeapNode *root)
    {
        makeCodesHelper(root, "");
    }

    string getEncodedText(const string &text)
    {
        string encodedText;
        for (char ch : text)
        {
            encodedText += codes[ch];
        }
        return encodedText;
    }

    string padEncodedText(const string &encodedText)
    {
        int extraPadding = 8 - (encodedText.length() % 8);
        string paddedText = encodedText;

        for (int i = 0; i < extraPadding; ++i)
        {
            paddedText += "0";
        }

        bitset<8> paddingInfo(extraPadding);
        return paddingInfo.to_string() + paddedText;
    }

    vector<uint8_t> getByteArray(const string &paddedEncodedText)
    {
        vector<uint8_t> byteArray;
        for (size_t i = 0; i < paddedEncodedText.length(); i += 8)
        {
            bitset<8> byte(paddedEncodedText.substr(i, 8));
            byteArray.push_back(byte.to_ulong());
        }
        return byteArray;
    }

public:
    Huffman(const string &inputPath) : path(inputPath) {}

    ~Huffman()
    {
        clearHeap();
    }

    string compress()
    {
        filesystem::path inputPath(path);
        string filename = inputPath.stem().string();
        string outputPath = "encoded/" + filename + ".bin";

        ifstream inputFile(path);
        ofstream outputFile(outputPath, ios::binary);

        if (!inputFile || !outputFile)
        {
            throw runtime_error("Unable to open files");
        }

        string text((istreambuf_iterator<char>(inputFile)),
                    istreambuf_iterator<char>());

        auto frequency = makeFrequencyDict(text);
        makeHeap(frequency);

        unique_ptr<HeapNode> root = mergeNodes();
        makeCodes(root.get());

        string encodedText = getEncodedText(text);
        string paddedEncodedText = padEncodedText(encodedText);
        auto byteArray = getByteArray(paddedEncodedText);

        outputFile.write(reinterpret_cast<char *>(byteArray.data()), byteArray.size());

        cout << "Compressed" << endl;
        return outputPath;
    }

    string removePadding(const std::string &paddedEncodedText)
    {
        // Extract first 8 bits representing padding information
        std::string paddedInfo = paddedEncodedText.substr(0, 8);

        // Convert binary padding info to integer
        int extraPadding = std::bitset<8>(paddedInfo).to_ulong();

        // Remove padding info and extra padding bits
        std::string encodedText = paddedEncodedText.substr(8);
        encodedText = encodedText.substr(0, encodedText.length() - extraPadding);

        return encodedText;
    }

    string decodeText(const std::string &encodedText)
    {
        std::string currentCode;
        std::string decodedText;

        for (char bit : encodedText)
        {
            currentCode += bit;

            // Check if current code exists in reverse mapping
            if (reverse_mapping.find(currentCode) != reverse_mapping.end())
            {
                char character = reverse_mapping[currentCode];
                decodedText += character;
                currentCode.clear();
            }
        }

        return decodedText;
    }

    string decompress(const std::string &inputPath)
    {
        // Generate output path
        std::filesystem::path inputFilePath(inputPath);
        std::string filename = inputFilePath.stem().string();
        std::string outputPath = "decoded/" + filename + "_decompressed.txt";

        // Open input binary file
        std::ifstream inputFile(inputPath, std::ios::binary);
        std::ofstream outputFile(outputPath);

        if (!inputFile || !outputFile)
        {
            throw std::runtime_error("Unable to open files");
        }

        // Read file byte by byte and convert to bit string
        std::string bitString;
        unsigned char byte;
        while (inputFile.read(reinterpret_cast<char *>(&byte), 1))
        {
            // Convert each byte to 8-bit binary string
            std::bitset<8> bits(byte);
            bitString += bits.to_string();
        }

        // Remove padding and decode
        std::string encodedText = removePadding(bitString);
        std::string decompressedText = decodeText(encodedText);

        // Write decompressed text
        outputFile << decompressedText;

        std::cout << "Decompressed" << std::endl;
        return outputPath;
    }

    unordered_map<char, double> calculateSymbolProbabilities(const string &text)
    {
        // First, get the frequency dictionary
        unordered_map<char, int> frequency = makeFrequencyDict(text);
        unordered_map<char, double> probabilities;

        // Total number of characters
        int totalChars = text.length();

        // Calculate probability for each character
        for (const auto &[ch, freq] : frequency)
        {
            probabilities[ch] = static_cast<double>(freq) / totalChars;
        }

        return probabilities;
    }

    // Optional: Method to print probabilities
    void printSymbolProbabilities(const unordered_map<char, double> &probabilities)
    {
        cout << "Symbol Probabilities:\n";
        for (const auto &[ch, prob] : probabilities)
        {
            // Handle special characters and print probability
            if (ch == '\n')
            {
                cout << "\\n: " << prob * 100 << "%\n";
            }
            else if (ch == ' ')
            {
                cout << "' ': " << prob * 100 << "%\n";
            }
            else
            {
                cout << ch << ": " << prob * 100 << "%\n";
            }
        }
    }
};
