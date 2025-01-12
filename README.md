# Huffman coding in C++ and analysis using Python

## **Huffman Coding: An Optimal Data Compression Technique**

Huffman coding is a fundamental lossless data compression algorithm that efficiently encodes data by assigning variable-length binary codes based on character frequencies. By creating a binary tree where more frequent characters receive shorter codes, the method minimizes overall data representation length. The algorithm constructs an optimal prefix-free encoding through a systematic process of frequency analysis, node combination, and code generation. Its elegance lies in transforming statistical data characteristics into a compact, efficient representation, making it a cornerstone technique in information theory and digital communication systems for reducing data transmission and storage requirements.

## Features:

- Symbol probabilities calculation
- Code word generation
- Encoding/Decoding
- Visualization using python
- Storing results in csv file for further analysis

## Dependencies

#### C++

```
 gnuplot-iostream
```

#### Python

```
Pandas
Matplotlib
seaborn
numpy
```

## Usage

- Run `main.cpp` and then use the resultant `compression_metrics.csv` file for python to run detailed analysis
