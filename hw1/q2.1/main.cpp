#include <MERSENNE_TWISTER.h>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

// a random number generator
MERSENNE_TWISTER twister(123456);

// prototypes
string whichColor(float p);

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
    cout << "Command line arguments were: " << endl;
    for (int x = 0; x < argc; x++) cout << argv[x] << " ";
    cout << endl;

    // you need exactly 8 arguments (1 program, 1 probability, 6 colors)
    if (argc != 8) {
        cout << "Usage: ./run probability red green blue red green blue"
             << endl;
        return -1;
    }

    // try to convert the probability into a float
    float probability = atof(argv[1]);

    // try to convert them into ints
    vector<int> colors(argc - 2);
    for (int x = 2; x < argc; x++) {
        colors[x - 2] = atoi(argv[x]);
    }

    cout << "Conversion results:" << endl;
    cout << "Probability of first color: " << probability << endl;
    cout << "R, G, B: " << colors[0] << ", " << colors[1] << ", " << colors[2]
         << endl;
    cout << "R, G, B: " << colors[3] << ", " << colors[4] << ", " << colors[5]
         << endl;

    cout << "Writing to file..." << endl;

    // allocate an array of floats (500 * 500 image, with 3 bytes (24 bits) for
    // RGB)
    float* values = new float[500 * 500 * 3];

    // for each pixel
    for (int x = 0; x < 500 * 500; x++) {
        // if the color is red
        if (whichColor(probability) == "one") {
            // skip ahead three at a time
            values[3 * x] = colors[0];
            values[3 * x + 1] = colors[1];
            values[3 * x + 2] = colors[2];

            // if the color is blue
        } else {
            // skip ahead three at a time
            values[3 * x] = colors[3];
            values[3 * x + 1] = colors[4];
            values[3 * x + 2] = colors[5];
        }
    }

    writePPM("noise.ppm", 500, 500, values);
}

string whichColor(float p) {
    if (twister.rand() <= p) {
        return "one";
    } else {
        return "two";
    }
}
