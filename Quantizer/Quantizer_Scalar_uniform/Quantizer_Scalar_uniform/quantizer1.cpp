#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


bool Scalar_uniform_quantization(string, string, int, int, int);
void Encoding(string, int, int, int, unsigned char**, unsigned char**, unsigned char**);


int main()
{
	/* set input path */
	string input_file = "./input/Lenna_512x512_original.raw";
	/* set output (encode file) path */
	string code_file  = "./output/Lenna_512x512_code";
	/* set input image's width & heigth */
	int input_width = 512;
	int input_height = 512;
	/* set encoded bits to 1 ~ 7 */ 
	int n_of_bits = 7;  

	if (Scalar_uniform_quantization(input_file, code_file, input_width, input_height, n_of_bits))
		cout << "Quantization success" << endl;
	else
		cout << "Quantization fail" << endl;

	return 0;
}

/* Scalar uniform quantize for input file and create encoded code file */
bool Scalar_uniform_quantization(string intput_file,  /* input image file     */
								 string code_file,    /* output code file     */
								 int width,           /* input image's width  */
								 int height,          /* input image's height */
								 int n_of_bits)       /* n-bits of code       */
{
	unsigned char** input_Rdata = new unsigned char* [height];  // 2D array to save input R raw data
	for (int i = 0; i < height; i++)							//
		input_Rdata[i] = new unsigned char[width];			    //	
	unsigned char** input_Gdata = new unsigned char* [height];  // 2D array to save input G raw data
	for (int i = 0; i < height; i++)							//
		input_Gdata[i] = new unsigned char[width];			    //	
	unsigned char** input_Bdata = new unsigned char* [height];  // 2D array to save input B raw data
	for (int i = 0; i < height; i++)							//
		input_Bdata[i] = new unsigned char[width];			    //

	unsigned char** output_Rdata = new unsigned char* [height]; // 2D array to save output R raw data
	for (int i = 0; i < height; i++)							//
		output_Rdata[i] = new unsigned char[width];			    //
	unsigned char** output_Gdata = new unsigned char* [height]; // 2D array to save output G raw data
	for (int i = 0; i < height; i++)							//
		output_Gdata[i] = new unsigned char[width];				//
	unsigned char** output_Bdata = new unsigned char* [height]; // 2D array to save output B raw data
	for (int i = 0; i < height; i++)							//
		output_Bdata[i] = new unsigned char[width];				//

	FILE* fin = fopen(intput_file.c_str(), "rb");       // Open input raw file
	if (fin == NULL) {                                  // If open fail,
		cout << "Error: Input file open fail" << endl;  // print error message
		exit(-1);                                       // and exit program
	}                                                   //
	for (int i = 0; i < height; i++)                    // Read input raw file data
	{                                                   //
		for (int j = 0; j < width; j++)                 //
		{                                               //
			input_Rdata[i][j] = fgetc(fin);             //
			input_Gdata[i][j] = fgetc(fin);             //
			input_Bdata[i][j] = fgetc(fin);             //
		}                                               //
	}                                                   //
	fclose(fin);                                        // Close input file stream

	unsigned char step = 256 / pow(2, n_of_bits);           // Get step size
	for (int i = 0; i < height; i++)                        // 
	{                                                       //
		for (int j = 0; j < width; j++)                     //
		{                                                   //
			output_Rdata[i][j] = input_Rdata[i][j] / step;  // Map input value to n-bits code
			output_Gdata[i][j] = input_Gdata[i][j] / step;  //
			output_Bdata[i][j] = input_Bdata[i][j] / step;  //
		}                                                   //
	}

	/* Encoding quantization code to output code file */
	Encoding(code_file, height, width, n_of_bits, output_Rdata, output_Gdata, output_Bdata);

	for (int i = 0; i < height; i++)   
		delete[] input_Rdata[i];			
	delete[] input_Rdata;				
	for (int i = 0; i < height; i++)   
		delete[] input_Gdata[i];			
	delete[] input_Gdata;				
	for (int i = 0; i < height; i++)  
		delete[] input_Bdata[i];			
	delete[] input_Bdata;				
	for (int i = 0; i < height; i++)
		delete[] output_Rdata[i];	   
	delete[] output_Rdata;
	for (int i = 0; i < height; i++)
		delete[] output_Gdata[i];
	delete[] output_Gdata;
	for (int i = 0; i < height; i++)
		delete[] output_Bdata[i];
	delete[] output_Bdata;

	return true;
} 

/* Encoding quantization code to output code file */
void Encoding(string code_file,              /* input image file     */
	          int height,                    /* input image's height */
			  int width,                     /* input image's width  */
			  int n_of_bits,                 /* n-bits of code       */
			  unsigned char** output_Rdata,  /* quantized R code     */
			  unsigned char** output_Gdata,  /* quantized R code     */
			  unsigned char** output_Bdata)  /* quantized R code     */
{
	FILE* fout = fopen(code_file.c_str(), "w+b");        // Open output code file 
	if (fout == NULL) {                                  // If open fail, 
		cout << "Error: code file create fail" << endl;  // print error message
		exit(-1);                                        // and exit program
	}                                   

	unsigned char buf = 0;                                 
	int remain_bit = 8;
	int diff = 8 - n_of_bits;                      
	for (int i = 0; i < height; i++)                     // 
	{                                                    //
		for (int j = 0; j < width; j++)                  //
		{                                                //
			unsigned int RGB_code = 0;                   // Gather each pixel's RGB code and align left
			unsigned int temp = 0;                       //  
			temp = temp | (output_Rdata[i][j] << diff);  // add R code 
			temp = temp << 24;                           //
			RGB_code = RGB_code | temp;                  //
			temp = 0;                                    //
			temp = temp | (output_Gdata[i][j] << diff);  // add G code  
			temp = temp << 16;                           //
			RGB_code = RGB_code | (temp << diff);        //
			temp = 0;                                    //
			temp = temp | (output_Bdata[i][j] << diff);  // add B code 
			temp = temp << 8;                            //
			RGB_code = RGB_code | (temp << 2 * diff);    //

			int code_size = 3 * n_of_bits;                    // number of bits to write
			while (code_size > 0)                             // Write RGB code to file
			{                                                 //
				buf = buf | (RGB_code >> (32 - remain_bit));  // fill buffer
				if (code_size >= remain_bit) {                // if buffer is full,                                                                // if remain bitstream >= remain bits in 1byte buffer(= buffer is full), 
					code_size -= remain_bit;                  // update number of bits to write                                          // remain bitstream size -= remain bits in buffer
					RGB_code = RGB_code << remain_bit;        // update RGB code and align left                                                         // remove bitstream which went to buffer
					fputc(buf, fout);                         // write buffer to file                                                             // write buffer to file
					buf = 0;                                  // reset buffer                                                  // reset buffer
					remain_bit = 8;                           //                                                 // 
				}                                             //                                                          // 
				else {                                        // if buffer isn't full,                                        // if remain bitstream < remain bits in 1byte buffer(= buffer isn't full), 
					remain_bit -= code_size;                  // save buffer                                                                // remain bits in buffer -= remain bitstream size
					code_size = 0;                            // and go next pixel                                                      // end loop
				}                                             //   
			}                                                 //   
		}                                                     //
	}                                                         // 
	fputc(buf, fout);                                         // write last buffer
	fclose(fout);                                             // Close output code file
}
