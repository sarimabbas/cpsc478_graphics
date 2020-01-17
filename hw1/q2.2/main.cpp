#include <cmath>
#include <iostream>
#include <vector>
#include <MERSENNE_TWISTER.h>

using namespace std;

// a random number generator
MERSENNE_TWISTER twister(123456);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void readPPM(const string& filename, int& xRes, int& yRes, float*& values)
{
  // try to open the file
  FILE *fp;
  fp = fopen(filename.c_str(), "rb");
  if (fp == NULL)
  {
    cout << " Could not open file \"" << filename.c_str() << "\" for reading." << endl;
    cout << " Make sure you're not trying to read from a weird location or with a " << endl;
    cout << " strange filename. Bailing ... " << endl;
    exit(0);
  }

  // get the dimensions
  fscanf(fp, "P6\n%d %d\n255\n", &xRes, &yRes);
  int totalCells = xRes * yRes;

  // grab the pixel values
  unsigned char* pixels = new unsigned char[3 * totalCells];
  fread(pixels, 1, totalCells * 3, fp);

  // copy to a nicer data type
  values = new float[3 * totalCells];
  for (int i = 0; i < 3 * totalCells; i++)
    values[i] = pixels[i];

  // clean up
  delete[] pixels;
  fclose(fp);
  cout << " Read in file " << filename.c_str() << endl;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void writePPM(const string& filename, const int xRes, const int yRes, const float* values)
{
  // copy to the data type PPM expects
  int totalCells = xRes * yRes;
  unsigned char* pixels = new unsigned char[3 * totalCells];
  for (int i = 0; i < 3 * totalCells; i++)
    pixels[i] = values[i];

  // try to open the file
  FILE *fp;
  fp = fopen(filename.c_str(), "wb");
  if (fp == NULL)
  {
    cout << " Could not open file \"" << filename.c_str() << "\" for writing." << endl;
    cout << " Make sure you're not trying to write from a weird location or with a " << endl;
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
void parseArgs(int& argc, char**& argv)
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

  // try to convert them into doubles
  vector<double> commandDoubles(argc);
  for (int x = 0; x < argc; x++)
    commandDoubles[x] = atof(argv[x]);

  cout << " I tried to convert each into double and got: " << endl;
  for (int x = 0; x < argc; x++)
    cout << commandDoubles[x] << " ";
  cout << endl;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  int xRes, yRes;
  float* values = NULL;

  if (argc != 2)
  {
    cout << " Expecting args " << argv[0] << " <input filename> " << endl;
    return 0;
  }

  readPPM(argv[1], xRes, yRes, values);

  // stomp off everything except red
  for (int y = 0; y < yRes; y++)
    for (int x = 0; x < xRes; x++)
    {
      int index = x + y * xRes;
      values[3 * index + 1] = 0;
      values[3 * index + 2] = 0;
    }
  
  writePPM("red.ppm", xRes, yRes, values);

  return 0;
}
