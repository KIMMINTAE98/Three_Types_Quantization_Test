#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


bool Vector_nonuniform_dequantization(string, string, string, string, int, int, int);


int main()
{
	/* set input path */
	string input_file    = "./input/Lenna_512x512_original.raw";
	/* set encode file path */
	string code_file     = "./output/Lenna_512x512_code";
	/* set codebook file path */
	string codebook_file = "./output/Lenna_512x512_codebook";
	/* set output path */
	string output_file = "./output/Lenna_512x512_reconstruct.raw";
	/* set input image's width & heigth */
	int input_width = 512;
	int input_height = 512;
	/* set encoded bits to 1 ~ 7 */
	int n_of_bits = 7;

	if (Vector_nonuniform_dequantization(input_file, code_file, output_file, codebook_file, input_width, input_height, n_of_bits))
		cout << "Dequantization success" << endl;
	else
		cout << "Dequantization fail" << endl;

	return 0;
}


/* Vector nonuniform dequantize input code file and reconstruct output image file  */
bool Vector_nonuniform_dequantization(string input_file,     /* input image file     */
									  string code_file,      /* input code file      */
									  string output_file,    /* output image file    */
									  string codebook_file,  /* intput codebook file */
									  int width,             /* input image's width  */
								      int height,            /* input image's height */
									  int n_of_bits)         /* n-bits of code       */
{
	FILE* fc = fopen(code_file.c_str(), "rb");         // Open input code file
	if (fc == NULL) {                                  // if file open fail, 
		cout << "Error: code file open fail" << endl;  // print error message
		exit(-1);                                      // and exit program
	}
	FILE* fcb = fopen(codebook_file.c_str(), "rb");        // Open input codebook file
	if (fcb == NULL) {                                     // if file open fail, 
		cout << "Error: codebook file open fail" << endl;  // print error message
		exit(-1);                                          // and exit program
	}
	FILE* fout = fopen(output_file.c_str(), "w+b");        // Open output image file 
	if (fout == NULL) {                                    // if  file  open fail, 
		cout << "Error: output file create fail" << endl;  // print error message
		exit(-1);                                          // and exit program
	}

	// Code to calculate MSE (1/3)
	FILE* fin = fopen(input_file.c_str(), "rb");
	if (fin == NULL) {
		cout << "Error: input file open fail" << endl;
		exit(-1);
	}


	/* Decode codebook file to get reconstruction vectors */
	int n_of_steps = pow(2, n_of_bits);                          // Get the number of step
	unsigned char** codebook = new unsigned char* [n_of_steps];  // Array to save codebook
	for (int i = 0; i < n_of_steps; i++)                         //
		codebook[i] = new unsigned char[3];                      //
	for (int i = 0; i < n_of_steps; i++)                         // 
	{                                                            // Get reconstruction vectors
		codebook[i][0] = fgetc(fcb);                             //
		codebook[i][1] = fgetc(fcb);                             //  
		codebook[i][2] = fgetc(fcb);                             //
	}                                                            //
	fclose(fcb);                                                 //

	/* Decode code file and reconstruct output file */
	unsigned int buf = fgetc(fc);                 // 32-bits buffer to read file
	buf = (buf << 8) | fgetc(fc);                 //
	buf = (buf << 8) | fgetc(fc);                 //
	buf = (buf << 8) | fgetc(fc);                 //
	int vaild_bits = 32;                          // the nubmer of vaild bits in buffer
	int sum = 0;                                  //
	for (int i = 0; i < height * width; i++)      // pixel's loop
	{                                             //
		unsigned char code = 0;                   //
		code = code | (buf >> (32 - n_of_bits));  // Get a quantized code
		fputc(codebook[code][0], fout);           // write reconsruction vector's R to file
		fputc(codebook[code][1], fout);           // write reconsruction vector's G to file  
		fputc(codebook[code][2], fout);           // write reconsruction vector's B to file
		buf = buf << n_of_bits;                   // Update buffer
		vaild_bits -= n_of_bits;                  // 
											   	  //
		if (vaild_bits < n_of_bits) {             // If buffer need to fill,
			unsigned int temp = fgetc(fc);        // fill buffer 16bit more
			temp = temp << 24;                    //
			buf = buf | (temp >> vaild_bits);     //
			temp = fgetc(fc);                     // 
			temp = temp << 16;                    //
			buf = buf | (temp >> vaild_bits);     //
			vaild_bits += 16;                     //
		}                                         
		
		// Code to calculate MSE (2/3)
		for (int j = 0; j < 3; j++)
		{
			unsigned char ori = fgetc(fin);
			int error = ori - codebook[code][j];
			sum += pow(error, 2);
		}
	}
	fclose(fc);
	fclose(fout);

	// Code to calculate MSE (3/3)
	fclose(fin);
	cout << "MSE = " << (double)sum / (height * width * 3) << endl;

	return true;
}
