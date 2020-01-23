#include <MERSENNE_TWISTER.h>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

// a random number generator
MERSENNE_TWISTER twister(123456);

// prototypes
void initPPM(float* values, int xRes, int yRes);
void bresenhamRasterizer(float* values, int x0, int y0, int x1, int y1,
                         int xRes, int yRes);
int getIndexFromCoordinate(int x, int y, int xRes);
void drawBlackPixel(int x, int y, int xRes, int yRes, float* values);
void runTests(float* values, int xRes, int yRes);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void writePPM(string filename, int xRes, int yRes, float* values) {
    int totalCells = xRes * yRes;
    unsigned char* pixels = new unsigned char[3 * totalCells];
    for (int i = 0; i < 3 * totalCells; i++) pixels[i] = values[i];

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

    fprintf(fp, "P6\n%d %d\n255\n", xRes, yRes);
    fwrite(pixels, 1, totalCells * 3, fp);
    fclose(fp);
    delete[] pixels;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
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

    // allocate space for the picture
    int xRes = 500;
    int yRes = 500;
    float* values = new float[3 * xRes * yRes];
    if (!values) {
        cout << "Could not allocate values" << endl;
        return -1;
    }

    // init ppm to red color background
    initPPM(values, xRes, yRes);

    // draw the line
    // slopes between 0 and 1
    bresenhamRasterizer(values, commandInts[1], commandInts[2], commandInts[3],
                        commandInts[4], xRes, yRes);
    // write PPM to file
    writePPM("drawline.ppm", xRes, yRes, values);

    delete[] values;
}

void initPPM(float* values, int xRes, int yRes) {
    for (int i = 0; i < xRes * yRes * 3; i += 3) {
        values[i] = 255;
        values[i + 1] = 0;
        values[i + 2] = 0;
    }
}

int getIndexFromCoordinate(int x, int y, int xRes) {
    int index = (y * xRes * 3) + (x * 3);
    return index;
}

void drawBlackPixel(int x, int y, int xRes, int yRes, float* values) {
    // if the origin was the bottom left
    // the reason for getting index (x, yRes - 1 - y) is to flip the y-axis
    // the index will always be 0 <= index <= 499
    // int index = getIndexFromCoordinate(x, (yRes - 1) - y, xRes)

    // if the origin is at the upper left
    int index = getIndexFromCoordinate(x, y, xRes);

    // set color values to black
    values[index] = 0;
    values[index + 1] = 0;
    values[index + 2] = 0;
}

// based on the reading in chapter 5 of the intro graphics book
void bresenhamRasterizer(float* values, int x0, int y0, int x1, int y1,
                         int xRes, int yRes) {
    // exchange endpoints if needed
    // if (x0 > x1) {
    //     int temp = x0;
    //     x0 = x1;
    //     x1 = temp;

    //     temp = y0;
    //     y0 = y1;
    //     y1 = temp;
    // }

    // get delta y and delta x
    int delta_y = y1 - y0;
    int delta_x = x1 - x0;

    if (delta_x > delta_y) {
        // the gradient is between 0 and 1
        // iterate along the x axis (left to right)

        // the initial y coord
        int j = y0;

        // calculate fault term
        float D = (float)delta_x / 2;

        // iterate
        for (int i = x0; i < x1; i++) {
            // output first black pixel
            drawBlackPixel(i, j, xRes, yRes, values);

            // update fault
            D = D - delta_y;

            if (D < 0) {
                j = j + 1;
                D = D + delta_x;
            }
        }
    } else {
        // the gradient is between 1 and infinity
        // iterate along the y axis
        // symmetrical

        // the initial x coord
        int i = x0;

        // calculate fault term
        float D = (float)delta_y / 2;

        // iterate
        for (int j = y0; j < y1; j++) {
            // output first black pixel
            drawBlackPixel(i, j, xRes, yRes, values);

            // update fault
            D = D - delta_x;

            if (D < 0) {
                i = i + 1;
                D = D + delta_y;
            }
        }
    }
}
