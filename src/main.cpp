#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <limits>
#include <chrono>
#include <fstream> 
#include <filesystem>

#define cimg_display 0
#include "CImg-3.5.4_pre04072515/CImg.h"

using namespace cimg_library;
using namespace std;

struct RGB {
    double r = 0.0, g = 0.0, b = 0.0;
};

class QuadtreeNode {
public:
    RGB avgColor;
    int x, y, width, height;
    bool isLeaf = false;

    QuadtreeNode *children[4] = {nullptr, nullptr, nullptr, nullptr}; // NW, NE, SW, SE
    int depth = 0;

    QuadtreeNode(int _x, int _y, int _w, int _h, int _d) : x(_x), y(_y), width(_w), height(_h), depth(_d) {}

    ~QuadtreeNode() {
        for (int i = 0; i < 4; ++i) {
            delete children[i];
        }
    }
};


void printWarning(string message) {
    cout << "\033[1;31m  !! " << message << "\033[0m\n";
}

void printCommand(string message) {
    cout << "\n\033[2m╭────\033[0m" << message << "\n\033[2m╰─\033[0;32m❯❯ \033[0m";
}

void printLine(string message) {
    cout << "\033[1;32m  ▓╟ " << message << "\033[0m\n";
}

void clearCinBuffer() {
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

long long getFileSizeStream(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streampos size = file.tellg();
    file.close();
    return static_cast<long long>(size);
}


RGB calculateAverageColor(const CImg<unsigned char>& image, int x, int y, int width, int height) {
    RGB avg = {};
    double numPixels = static_cast<double>(width) * height;
    if (numPixels == 0) return avg;

    double r_sum = 0.0, g_sum = 0.0, b_sum = 0.0;

    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            r_sum += image(i, j, 0, 0);
            g_sum += image(i, j, 0, 1);
            b_sum += image(i, j, 0, 2);
        }
    }
    avg.r = r_sum / numPixels;
    avg.g = g_sum / numPixels;
    avg.b = b_sum / numPixels;
    return avg;
}

double calculateErrorVariance(const CImg<unsigned char>& image, int x, int y, int width, int height, const RGB& avgColor) {
    double numPixels = static_cast<double>(width) * height;
    if (numPixels <= 1) return 0.0;

    double r_var_sum = 0.0, g_var_sum = 0.0, b_var_sum = 0.0;

    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            double r_diff = image(i, j, 0, 0) - avgColor.r;
            double g_diff = image(i, j, 0, 1) - avgColor.g;
            double b_diff = image(i, j, 0, 2) - avgColor.b;
            r_var_sum += r_diff * r_diff;
            g_var_sum += g_diff * g_diff;
            b_var_sum += b_diff * b_diff;
        }
    }

    double r_variance = r_var_sum / numPixels;
    double g_variance = g_var_sum / numPixels;
    double b_variance = b_var_sum / numPixels;

    return (r_variance + g_variance + b_variance) / 3.0;
}

double calculateErrorMAD(const CImg<unsigned char>& image, int x, int y, int width, int height, const RGB& avgColor) {
    double numPixels = static_cast<double>(width) * height;
    if (numPixels == 0) return 0.0;

    double r_mad_sum = 0.0, g_mad_sum = 0.0, b_mad_sum = 0.0;

    int x_end = min(x + width, image.width());
    int y_end = min(y + height, image.height());

    for (int j = y; j < y_end; ++j) {
        for (int i = x; i < x_end; ++i) {
            r_mad_sum += abs(image(i, j, 0, 0) - avgColor.r);
            g_mad_sum += abs(image(i, j, 0, 1) - avgColor.g);
            b_mad_sum += abs(image(i, j, 0, 2) - avgColor.b);
        }
    }

    numPixels = static_cast<double>(x_end - x) * (y_end - y);
    if (numPixels <= 0) return 0.0;

    double r_mad = r_mad_sum / numPixels;
    double g_mad = g_mad_sum / numPixels;
    double b_mad = b_mad_sum / numPixels;

    return (r_mad + g_mad + b_mad) / 3.0;
}

double calculateErrorMaxDiff(const CImg<unsigned char>& image, int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return 0.0;
    unsigned char minR = 255, maxR = 0;
    unsigned char minG = 255, maxG = 0;
    unsigned char minB = 255, maxB = 0;
    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            unsigned char r = image(i, j, 0, 0);
            unsigned char g = image(i, j, 0, 1);
            unsigned char b = image(i, j, 0, 2);
            if (r < minR) minR = r;
            if (r > maxR) maxR = r;
            if (g < minG) minG = g;
            if (g > maxG) maxG = g;
            if (b < minB) minB = b;
            if (b > maxB) maxB = b;
        }
    }
    double DR = static_cast<double>(maxR - minR);
    double DG = static_cast<double>(maxG - minG);
    double DB = static_cast<double>(maxB - minB);
    double DRGB = (DR + DG + DB) / 3.0;
    return DRGB;
}

double calculateEntropy(const CImg<unsigned char>& image, int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return 0.0;
    std::vector<int> freqR(256, 0), freqG(256, 0), freqB(256, 0);
    int totalPixels = width * height;
    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            unsigned char r = image(i, j, 0, 0);
            unsigned char g = image(i, j, 0, 1);
            unsigned char b = image(i, j, 0, 2);
            freqR[r]++;
            freqG[g]++;
            freqB[b]++;
        }
    }
    auto calculateChannelEntropy = [&](const std::vector<int>& freq) -> double {
        double entropy = 0.0;
        for (int i = 0; i < 256; ++i) {
            if (freq[i] > 0) {
                double probability = static_cast<double>(freq[i]) / totalPixels;
                entropy -= probability * std::log2(probability);
            }
        }
        return entropy;
    };
    double entropyR = calculateChannelEntropy(freqR);
    double entropyG = calculateChannelEntropy(freqG);
    double entropyB = calculateChannelEntropy(freqB);
    double entropyRGB = (entropyR + entropyG + entropyB) / 3.0;
    return entropyRGB;
}

long long nodeCount = 0;
int maxDepth = 0;

QuadtreeNode* buildQuadtree(const CImg<unsigned char>& image, int x, int y, int width, int height,
                            double threshold, int minBlockSize, int currentDepth, int errorMethod) {

    nodeCount++;
    if (currentDepth > maxDepth) {
        maxDepth = currentDepth;
    }

    QuadtreeNode* node = new QuadtreeNode(x, y, width, height, currentDepth);

    node->avgColor = calculateAverageColor(image, x, y, width, height);

    double error = 0.0;
    if (errorMethod == 1) {
        error = calculateErrorVariance(image, x, y, width, height, node->avgColor);
    } else if (errorMethod == 2) { 
        error = calculateErrorMAD(image, x, y, width, height, node->avgColor);
    } else if (errorMethod == 3) {
        error = calculateErrorMaxDiff(image, x, y, width, height);
    } else if (errorMethod == 4) {
        error = calculateEntropy(image, x, y, width, height);
    } else if (errorMethod == 5) {
        // TODO
        cout << "SSIM" << endl;
        error = 0;
    } else {
        printWarning("Metode error tidak valid!");
        error = 0;
    }


    int nextWidth = width / 2;
    int nextHeight = height / 2;
    bool subBlocksTooSmall = (nextWidth == 0 || nextHeight == 0);


    if (error < threshold || (width * height) <= minBlockSize || subBlocksTooSmall ) {
        node->isLeaf = true;
    } else {
        node->isLeaf = false;

        int w1 = nextWidth;
        int h1 = nextHeight;
        int w2 = width - w1;
        int h2 = height - h1;

        if (w1 > 0 && h1 > 0)
            node->children[0] = buildQuadtree(image, x, y, w1, h1, threshold, minBlockSize, currentDepth + 1, errorMethod); // NW
        if (w2 > 0 && h1 > 0)
            node->children[1] = buildQuadtree(image, x + w1, y, w2, h1, threshold, minBlockSize, currentDepth + 1, errorMethod); // NE
        if (w1 > 0 && h2 > 0)
            node->children[2] = buildQuadtree(image, x, y + h1, w1, h2, threshold, minBlockSize, currentDepth + 1, errorMethod); // SW
        if (w2 > 0 && h2 > 0)
            node->children[3] = buildQuadtree(image, x + w1, y + h1, w2, h2, threshold, minBlockSize, currentDepth + 1, errorMethod); // SE
    }

    return node;
}

void reconstructImage(CImg<unsigned char>& outputImage, const QuadtreeNode* node) {
    if (node == nullptr) {
        return;
    }

    if (node->isLeaf) {
        unsigned char color[3];
        color[0] = static_cast<unsigned char>(max(0.0, min(255.0, node->avgColor.r)));
        color[1] = static_cast<unsigned char>(max(0.0, min(255.0, node->avgColor.g)));
        color[2] = static_cast<unsigned char>(max(0.0, min(255.0, node->avgColor.b)));

        // CImg draw_rectangle(x0, y0, x1, y1, color, opacity)
        outputImage.draw_rectangle(node->x, node->y,
                                   node->x + node->width - 1, node->y + node->height - 1,
                                   color, 1.0f);
    } else {
        for (int i = 0; i < 4; ++i) {
            reconstructImage(outputImage, node->children[i]);
        }
    }
}


int main() {
    string inputFile, outputFile = "test/default.png";
    string errorStr = "";
    int errorMethodChoice;
    double threshold, maxThreshold;
    int minBlockSize;
    CImg<unsigned char> inputImage;

    cout << endl;
    printLine("========== Kompresi Gambar Quadtree ==========");
    cout << "\033[1;32m  ▓╟ Current Working Directory: " << std::filesystem::current_path() << "\033[0m\n";
    
    while (true) {
        printCommand("Masukkan path gambar input");
        getline(cin, inputFile);

        try {
            CImg<unsigned char> testImg(inputFile.c_str());
            inputImage = testImg;
            if (inputImage.spectrum() != 3) {
                printWarning("Gambar input tidak memiliki 3 channel warna (RGB). Program mungkin tidak berfungsi benar.");
            }
            break;
        } catch (const cimg_library::CImgException& e) {
            printWarning("Error: Gambar tidak dikenali.");
            continue;
        }
    }
    long long inputSize = getFileSizeStream(inputFile);

    while (true) {
        printCommand("Pilih metode error (1=Variance, 2=MAD, 3=MaxDiff, 4=Entropy, 5=SSIM)");
        cin >> errorMethodChoice;

        if (cin.fail()) {
            printWarning("Error: Masukan harus berupa angka.");
            cin.clear();
            clearCinBuffer();
            continue;
        }
        
        if (errorMethodChoice >= 1 && errorMethodChoice <= 5) {
            break;
        } else {
            printWarning("Error: Pilihan metoda harus antara 1 hingga 5 (inklusif).");
        }
    }
    
    switch (errorMethodChoice) {
        case 1: 
            maxThreshold = 65025.0; 
            errorStr = "Variance";
            break;
        case 2: 
            maxThreshold = 255.0; 
            errorStr = "Mean Absolute Deviation (MAD)";
            break;
        case 3: 
            maxThreshold = 255.0; 
            errorStr = "Max Pixel Difference";
            break;
        case 4: 
            maxThreshold = 8.0; 
            errorStr = "Entropy";
            break;
        case 5: 
            maxThreshold = 1000.0; // TODO
            errorStr = "SSIM";
            break;
    }

    while (true) {
        printCommand("Masukkan nilai threshold untuk " + errorStr +
                    " (rentang efektif: 0.0 - " + to_string(maxThreshold) + ")");
        cin >> threshold;
        
        if (cin.fail()) {
            printWarning("Error: Masukan harus berupa angka.");
            cin.clear();
            clearCinBuffer();
            continue;
        }
        
        if (threshold >= 0.0) {
            if (threshold > maxThreshold) {
                printWarning("Nilai di atas " + to_string(maxThreshold) + " kemungkinan tidak akan berpengaruh pada kompresi");
            }
            break;
        } else {
            printWarning("Error: Threshold tidal boleh negatif.");
        }
    }
    
    while (true) {
        printCommand("Masukkan ukuran blok minimum (>= 1)");
        cin >> minBlockSize;
        
        if (cin.fail()) {
            printWarning("Error: Masukan harus berupa angka integer.");
            cin.clear();
            clearCinBuffer();
            continue;
        }
        
        if (minBlockSize >= 1) {
            break; 
        } else {
            printWarning("Error: Ukuran blok minimum harus 1 atau lebih besar.");
        }
    }

    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    printCommand("Masukkan path gambar output");
    getline(cin, outputFile);

    // --- Process ---
    cout << "\n";
    try {
        printLine("Memulai Kompresi...");
        CImg<unsigned char> outputImage(inputImage.width(), inputImage.height(), 1, 3, 0);
        
        auto startTime = chrono::high_resolution_clock::now();
        
        nodeCount = 0;
        maxDepth = 0;
        
        QuadtreeNode* root = buildQuadtree(inputImage, 0, 0, inputImage.width(), inputImage.height(),
        threshold, minBlockSize, 0, errorMethodChoice);
        
        reconstructImage(outputImage, root);
        
        auto endTime = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> duration = endTime - startTime;
        printLine("Kompresi Selesai.");
        
        printLine("Menyimpan file gambar...");
        size_t dotPos = outputFile.find_last_of(".");
        if (dotPos == string::npos) {
            outputFile += ".png";
            printWarning("Ekstensi file output tidak dispesifikasi. Menyimpan sebagai PNG.");
        }
        outputImage.save(outputFile.c_str());
        printLine("Gambar tersimpan.");

        printLine("");
        
        // --- Statistik ---
        printLine("--- Statistik ---");
        printLine("Path gambar output     : " + outputFile);
        printLine("Waktu eksekusi         : " + to_string(duration.count()) + " ms");
        
        printLine("Resolusi Gambar        : " + to_string(inputImage.width()) + "x" + to_string(inputImage.height()));

        printLine("Metode error           : " + errorStr);
        printLine("Threshold              : " + to_string(threshold));
        printLine("Blok Minimum           : " + to_string(minBlockSize));
        printLine("Ukuran sebelum         : " + to_string(inputSize / (1024.0 * 1024.0)) + " MiB (" + 
                                                to_string(inputSize) + " bytes)");
        
        long long outputSize = getFileSizeStream(outputFile);
        if (outputSize >= 0) {
            printLine("Ukuran sesudah         : " + to_string(outputSize / (1024.0 * 1024.0)) + " MiB (" +
                                                  to_string(outputSize) + " bytes)");

            if (inputSize > 0) {
                double compressionRatio = static_cast<double>(outputSize) / inputSize;
                printLine("Rasio kompresi         : " + to_string(compressionRatio));

                double compressionPercentage = (1.0 - compressionRatio) * 100.0;
                printLine("Persentase kompresi    : " + to_string(compressionPercentage) + " %");
            } else {
                printLine("Rasio kompresi         : N/A (Ukuran input 0 atau error)");
                printLine("Persentase kompresi    : N/A");
            }
        } else {
            printWarning("Tidak dapat membaca ukuran file output.");
        }

        printLine("Kedalaman pohon maks   : " + to_string(maxDepth));
        printLine("Jumlah simpul total    : " + to_string(nodeCount));
        cout << endl;

        delete root;

    } catch (CImgException &e) {
        printWarning("Error CImg: " + std::string(e.what()));
        return 1;
    } catch (exception &e) {
        printWarning("Error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}