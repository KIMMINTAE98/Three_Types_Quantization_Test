#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


bool Scalar_uniform_dequantization(string, string, string, int, int, int);


int main()
{
	/* set input path */
	string input_file  = "./input/Lenna_512x512_original.raw";
	/* set encode file path */
	string code_file   = "./output/Lenna_512x512_code";
	/* set output path */
	string output_file = "./output/Lenna_512x512_reconstruct.raw";
	/* set input image's width & heigth */
	int input_width = 512;
	int input_height = 512;
	/* set encoded bits to 1 ~ 7 */
	int n_of_bits = 7;

	if (Scalar_uniform_dequantization(input_file, code_file, output_file, input_width, input_height, n_of_bits))
		cout << "Dequantization success" << endl;
	else
		cout << "Dequantization fail" << endl;

	return 0;
}


/* Scalar uniform dequantize input code file and reconstruct output image file  */
bool Scalar_uniform_dequantization(string input_file,   /* input image file     */
								   string code_file,    /* input code file      */
								   string output_file,  /* output image file    */
								   int width,           /* input image's width  */
								   int height,          /* input image's height */
								   int n_of_bits)       /* n-bits of code       */
{
	FILE* fc = fopen(code_file.c_str(), "rb");         // Open input code file
	if (fc == NULL) {                                  // if input code file open fail, 
		cout << "Error: code file open fail" << endl;  // print error message
		exit(-1);                                      // and exit program
	}
	FILE* fout = fopen(output_file.c_str(), "w+b");        // Open output image file 
	if (fout == NULL) {                                    // if output image file  open fail, 
		cout << "Error: output file create fail" << endl;  // print error message
		exit(-1);                                          // and exit program
	}
	/*
	// Code to calculate MSE (1/3)
	FILE* fin = fopen(input_file.c_str(), "rb");        
	if (fin == NULL) {                                
		cout << "Error: input file open fail" << endl; 
		exit(-1);                                      
	}
	*/
	                                                    // Decode code file and reconstruct output file
	unsigned char step = 256 / pow(2, n_of_bits);       // get step size
	unsigned int buf = fgetc(fc);                       // fill 32bit buffer
	buf = (buf << 8) | fgetc(fc);                       //
	buf = (buf << 8) | fgetc(fc);                       //
	buf = (buf << 8) | fgetc(fc);                       //
	int remain_bit = 32;                                //
	int sum = 0;                                        //
	for (int i = 0; i < height; i++)                    // height loop
	{                                                   //
		for (int j = 0; j < width * 3; j += 3)          // width loop
		{                                               //
			for (int k = 0; k < 3; k++)                 // RGB loop
			{                                           //
				unsigned char rec = 0;                  // get reconstuction level
				rec = rec | (buf >> (32 - n_of_bits));  //
				rec = rec * step + (step / 2);          //                                            
				fputc(rec, fout);                       // write reconstuction value
				buf = buf << n_of_bits;                 // update buffer
				remain_bit -= n_of_bits;                // 
														//
				if (remain_bit < n_of_bits) {           // if buffer need to fill,
					unsigned int temp = fgetc(fc);      // fill buffer 8bit more
					temp = temp << 24;                  //
					buf = buf | (temp >> remain_bit);   //
					remain_bit += 8;                    //
				}                                       //
				/*
				// Code to calculate MSE (2/3)
				unsigned char ori = fgetc(fin);         
				int error = ori - rec;                  
				sum += pow(error, 2);                   
				*/
			}
		}
	}
	fclose(fc);
	fclose(fout);                                             
	
	/* 
	// Code to calculate MSE (3/3)
	fclose(fin);
	cout << "MSE = " << (double) sum / (height * width * 3) << endl;
	*/

	return true;
}
