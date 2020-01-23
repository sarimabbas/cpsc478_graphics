#include <MERSENNE_TWISTER.h>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

// a random number generator
MERSENNE_TWISTER twister(123456);

// prototypes
int getPixelIndexAtCoord(int xCoord, int yCoord, int xRes);
void getAverageOfSurroundingPixels(int xCoord, int yCoord, int xRes, int yRes,
                                   float* values, float& red, float& green,
                                   float& blue);
void applyBlur(int xRes, int yRes, float* values, float* out);
void runTests();

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void readPPM(const string& filename, int& xRes, int& yRes, float*& values) {
    // try to open the file
    FILE* fp;
    fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        cout << " Could not open file \"" << filename.c_str()
             << "\" for reading." << endl;
        cout << " Make sure you're not trying to read from a weird location or "
                "with a "
             << endl;
        cout << " strange filename. Bailing ... " << endl;
        exit(0);
    }

    // get the dimensions
    fscanf(fp, "P6\n%d %d\n255\n", &xRes, &yRes);
    int totalCells = xRes * yRes;

    cout << "Total cells : " << totalCells << endl;

    // grab the pixel values
    unsigned char* pixels = new unsigned char[3 * totalCells];
    fread(pixels, 1, totalCells * 3, fp);

    // copy to a nicer data type
    values = new float[3 * totalCells];
    for (int i = 0; i < 3 * totalCells; i++) values[i] = pixels[i];

    // clean up
    delete[] pixels;
    fclose(fp);
    cout << " Read in file " << filename.c_str() << endl;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void writePPM(const string& filename, const int xRes, const int yRes,
              const float* values) {
    // copy to the data type PPM expects
    int totalCells = xRes * yRes;
    unsigned char* pixels = new unsigned char[3 * totalCells];
    for (int i = 0; i < 3 * totalCells; i++) pixels[i] = values[i];

    // try to open the file
    FILE* fp;
    fp = fopen(filename.c_str(), "wb");
    if (fp == NULL) {
        cout << " Could not open file \"" << filename.c_str()
             << "\" for writing." << endl;
        cout << " Make sure you're not trying to write from a weird location "
                "or with a "
             << endl;
        cout << " strange filename. Bailing ... " << endl;
        exit(0);
    }

    // write what PPM expects
    fprintf(fp, "P6\n%d %d\n255\n", xRes, yRes);
    fwrite(pixels, 1, totalCells * 3, fp);

    // clean up
    fclose(fp);
    delete[] pixels;
    cout << " Wrote out file " << filename.c_str() << endl;
}

///////////////////////////////////////////////////////////////////////
// Example of parsing command line arguments
///////////////////////////////////////////////////////////////////////
void parseArgs(int& argc, char**& argv) {
    // Print out the command line args
    cout << " Command line arguments were: " << endl;
    for (int x = 0; x < argc; x++) cout << argv[x] << " ";
    cout << endl;

    // try to convert them into ints
    vector<int> commandInts(argc);
    for (int x = 0; x < argc; x++) commandInts[x] = atoi(argv[x]);

    cout << " I tried to convert each into an integer and got: " << endl;
    for (int x = 0; x < argc; x++) cout << commandInts[x] << " ";
    cout << endl;

    // try to convert them into doubles
    vector<double> commandDoubles(argc);
    for (int x = 0; x < argc; x++) commandDoubles[x] = atof(argv[x]);

    cout << " I tried to convert each into double and got: " << endl;
    for (int x = 0; x < argc; x++) cout << commandDoubles[x] << " ";
    cout << endl;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    int xRes, yRes;
    float* values = NULL;

    runTests();

    if (argc != 2) {
        cout << " Expecting args " << argv[0] << " <input filename> " << endl;
        return -1;
    }

    readPPM(argv[1], xRes, yRes, values);

    // process the image and apply the filter
    float* out = new float[3 * xRes * yRes];
    applyBlur(xRes, yRes, values, out);

    writePPM("filtered.ppm", xRes, yRes, out);

    return 0;
}

void applyBlur(int xRes, int yRes, float* values, float* out) {
    for (int y = 0; y < yRes; y++) {
        for (int x = 0; x < xRes; x++) {
            float red = 0;
            float green = 0;
            float blue = 0;
            // get average of surrounding pixels into r,g,b vars
            getAverageOfSurroundingPixels(x, y, xRes, yRes, values, red, green,
                                          blue);
            // put that as the value
            int index = getPixelIndexAtCoord(x, y, xRes);
            // int index = 3 * (x + y * xRes);
            out[index] = red;
            out[index + 1] = green;
            out[index + 2] = blue;
        }
    }
}

void getAverageOfSurroundingPixels(int xCoord, int yCoord, int xRes, int yRes,
                                   float* values, float& red, float& green,
                                   float& blue) {
    int index;
    // 5 pixels in the x direction
    for (int i = xCoord - 2; i < xCoord + 3; i++) {
        // 5 pixels in the y direction
        for (int j = yCoord - 2; j < yCoord + 3; j++) {
            index = getPixelIndexAtCoord(i, j, xRes);
            if (index >= 0 && index < (xRes * yRes * 3)) {
                red += values[index];
                green += values[index + 1];
                blue += values[index + 1];
            } else {
                red += 0;
                green += 0;
                blue += 0;
            }
        }
    }
    // take averages
    red = red / 25;
    green = green / 25;
    blue = blue / 25;
}

// xCoord is the column
// yCoord is the row
// e.g. we want (x, y) == (0, 0) to give an index of 0
// e.g. we want (x, y) == (1, 0) to give an index of 3
// e.g. we want (x, y) == (0, 1) to give an index of 300
int getPixelIndexAtCoord(int xCoord, int yCoord, int xRes) {
    int index = (yCoord * xRes * 3) + (xCoord * 3);
    return index;
}

void runTests() { getPixelIndexAtCoord(50, 50, 100); }