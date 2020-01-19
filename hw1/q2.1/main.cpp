#include <cmath>
#include <iostream>
#include <vector>
#include <MERSENNE_TWISTER.h>

using namespace std;

// a random number generator
MERSENNE_TWISTER twister(123456);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void writePPM(string filename, int xRes, int yRes, float* values)
{
  int totalCells = xRes * yRes;
  unsigned char* pixels = new unsigned char[3 * totalCells];
  for (int i = 0; i < 3 * totalCells; i++)
    pixels[i] = values[i];

  FILE *fp;
  fp = fopen(filename.c_str(), "wb");
  if (fp == NULL)
  {
    cout << " Could not open file \"" << filename.c_str() << "\" for writing." << endl;
    cout << " Make sure you're not trying to write from a weird location or with a " << endl;
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
int main(int argc, char** argv)
{
  // Print out the command line args
  cout << " Command line arguments were: " << endl;
  for (int x = 0; x < argc; x++)
    cout << argv[x] << " ";
  cout << endl;

  // try to convert them into ints
  vector<int> commandInts(argc);
  for (int x = 0; x < argc; x++)
    commandInts[x] = atoi(argv[x]);

  cout << " I tried to convert each into an integer and got: " << endl;
  for (int x = 0; x < argc; x++)
    cout << commandInts[x] << " ";
  cout << endl;

  float* values = new float[500 * 500 * 3];

  for (int x = 0; x < 500 * 500; x++)
  {
    values[3 * x] = 255;
    values[3 * x + 1] = 0;
    values[3 * x + 2] = 0;
  }

  writePPM("random.ppm", 500, 500, values);
}
