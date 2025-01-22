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
#include <algorithm>
#include <cctype>

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

        HeapNode(char ch, int freq) : character(ch), frequency(freq) {}

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
    priority_queue<HeapNode *, vector<HeapNode *>, HeapNode::compare> heap;
    unordered_map<char, string> codes;
    unordered_map<string, char> reverse_mapping;

    void clearHeap()
    {
        while (!heap.empty())
        {
            delete heap.top();
            heap.pop();
        }
    }

    unordered_map<char, int> makeFrequencyDict(const string &text) const
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

    string getEncodedText(const string &text) const
    {
        string encodedText;
        for (char ch : text)
        {
            encodedText += codes.at(ch);
        }
        return encodedText;
    }

    string padEncodedText(const string &encodedText) const
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

    vector<uint8_t> getByteArray(const string &paddedEncodedText) const
    {
        vector<uint8_t> byteArray;
        for (size_t i = 0; i < paddedEncodedText.length(); i += 8)
        {
            bitset<8> byte(paddedEncodedText.substr(i, 8));
            byteArray.push_back(static_cast<uint8_t>(byte.to_ulong()));
        }
        return byteArray;
    }

public:
    explicit Huffman(const string &inputPath) : path(inputPath) {}

    ~Huffman()
    {
        clearHeap();
    }
    string compress()
    {
        filesystem::path inputPath(path);
        string filename = inputPath.stem().string();
        string outputPath = "encoded/" + filename + ".bin";

        ifstream inputFile(path, ios::binary);        // Open input file in binary mode
        ofstream outputFile(outputPath, ios::binary); // Open output file in binary mode

        if (!inputFile)
        {
            throw runtime_error("Unable to open input file: " + path);
        }

        if (!outputFile)
        {
            throw runtime_error("Unable to create output file: " + outputPath);
        }

        // Read the entire file into a string
        string text((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());

        // Debug: Check text length before filtering
        cout << "Original text length: " << text.length() << endl;

        // Filter out non-printable ASCII characters
        text.erase(remove_if(text.begin(), text.end(), [](unsigned char c)
                             {
                                 return !isascii(c) || !isprint(c); // Keep only printable ASCII characters
                             }),
                   text.end());

        // Debug: Check text length after filtering
        cout << "Filtered text length: " << text.length() << endl;

        if (text.empty())
        {
            throw runtime_error("Filtered text is empty. Ensure the input file contains valid printable ASCII characters.");
        }

        // Proceed with frequency calculation, heap creation, and encoding
        auto frequency = makeFrequencyDict(text);
        makeHeap(frequency);

        unique_ptr<HeapNode> root = mergeNodes();
        makeCodes(root.get());

        string encodedText = getEncodedText(text);
        string paddedEncodedText = padEncodedText(encodedText);
        auto byteArray = getByteArray(paddedEncodedText);

        outputFile.write(reinterpret_cast<char *>(byteArray.data()), byteArray.size());

        cout << "Compression successful: " << outputPath << endl;
        return outputPath;
    }

    string removePadding(const string &paddedEncodedText) const
    {
        string paddingInfo = paddedEncodedText.substr(0, 8);
        int extraPadding = bitset<8>(paddingInfo).to_ulong();
        string encodedText = paddedEncodedText.substr(8);
        encodedText = encodedText.substr(0, encodedText.length() - extraPadding);
        return encodedText;
    }

    string decodeText(const string &encodedText) const
    {
        string currentCode;
        string decodedText;

        for (char bit : encodedText)
        {
            currentCode += bit;
            if (reverse_mapping.find(currentCode) != reverse_mapping.end())
            {
                char character = reverse_mapping.at(currentCode);
                decodedText += character;
                currentCode.clear();
            }
        }

        return decodedText;
    }

    string decompress(const string &inputPath)
    {
        filesystem::path inputFilePath(inputPath);
        string filename = inputFilePath.stem().string();
        string outputPath = "decoded/" + filename + "_decompressed.txt";

        ifstream inputFile(inputPath, ios::binary);
        ofstream outputFile(outputPath);

        if (!inputFile)
        {
            throw runtime_error("Unable to open input file: " + inputPath);
        }

        if (!outputFile)
        {
            throw runtime_error("Unable to create output file: " + outputPath);
        }

        string bitString;
        unsigned char byte;
        while (inputFile.read(reinterpret_cast<char *>(&byte), 1))
        {
            bitset<8> bits(byte);
            bitString += bits.to_string();
        }

        string encodedText = removePadding(bitString);
        string decompressedText = decodeText(encodedText);

        outputFile << decompressedText;

        cout << "Decompression successful: " << outputPath << endl;
        return outputPath;
    }

    unordered_map<char, double> calculateSymbolProbabilities(const string &text) const
    {
        auto frequency = makeFrequencyDict(text);
        unordered_map<char, double> probabilities;

        int totalChars = text.length();
        for (const auto &[ch, freq] : frequency)
        {
            probabilities[ch] = static_cast<double>(freq) / totalChars;
        }

        return probabilities;
    }

    unordered_map<char, string> getCodeWords() const
    {
        return codes;
    }
};
