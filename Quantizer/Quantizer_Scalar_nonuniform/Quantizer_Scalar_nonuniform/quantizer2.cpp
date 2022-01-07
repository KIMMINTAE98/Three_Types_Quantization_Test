#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


bool Scalar_nonuniform_quantization(string, string, string, int, int, int, int);
void Encoding(string, int, int, int, unsigned char**, unsigned char**, unsigned char**);
void SaveCodebook(string, int, unsigned char*, unsigned char*, unsigned char*);


int main()
{
	/* set input path */
	string input_file	 = "./input/Lenna_512x512_original.raw";
	/* set output (encode file) path */
	string code_file	 = "./output/Lenna_512x512_code";
	/* set output (codebook file) path */
	string codebook_file = "./output/Lenna_512x512_codebook";
	/* set input image's width & heigth */
	int input_width = 512;
	int input_height = 512;
	/* set encoded bits to 1 ~ 7 */
	int n_of_bits = 7;
	/* set Lloyd-Max's iteration */
	int n_of_iter = 2;

	if (Scalar_nonuniform_quantization(input_file, code_file, codebook_file, input_width, input_height, n_of_bits, n_of_iter))
		cout << "Quantization success" << endl;
	else
		cout << "Quantization fail" << endl;

	return 0;
}

/* Scalar non-uniform quantize for input file and create codebook & encoded code file */
bool Scalar_nonuniform_quantization(string intput_file,    /* input image file        */
									string code_file,      /* output code file        */
									string codebook_file,  /* output code file        */
									int width,             /* input image's width     */
									int height,            /* input image's height    */
									int n_of_bits,         /* n-bits of code          */
								    int n_of_iter)         /* the number of iteration */
{
	unsigned char** input_Rdata = new unsigned char* [height];  // 2D array to save input R raw data
	unsigned char** input_Gdata = new unsigned char* [height];  // 2D array to save input G raw data
	unsigned char** input_Bdata = new unsigned char* [height];  // 2D array to save input B raw data
	for (int i = 0; i < height; i++) 							//
	{ 				                              			    //
		input_Rdata[i] = new unsigned char[width];			    //	
		input_Gdata[i] = new unsigned char[width];			    //	
		input_Bdata[i] = new unsigned char[width];			    //
	} 				                              			    //

	unsigned char** output_Rdata = new unsigned char* [height]; // 2D array to save output R raw data
	unsigned char** output_Gdata = new unsigned char* [height]; // 2D array to save output G raw data
	unsigned char** output_Bdata = new unsigned char* [height]; // 2D array to save output B raw data
	for (int i = 0; i < height; i++)							//
	{ 				                              			    //
		output_Rdata[i] = new unsigned char[width];			    //
		output_Gdata[i] = new unsigned char[width];				//
		output_Bdata[i] = new unsigned char[width];				//
	} 				                              			    //

	int R_histogram[256] = { 0, };  // Array to save R data's histogram
	int G_histogram[256] = { 0, };  // Array to save G data's histogram
	int B_histogram[256] = { 0, };  // Array to save B data's histogram

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
			R_histogram[input_Rdata[i][j]]++;           // Count historam
			G_histogram[input_Gdata[i][j]]++;           //
			B_histogram[input_Bdata[i][j]]++;           //
		}                                               //
	}                                                   //
	fclose(fin);                                        // Close input file stream

	int n_of_steps = pow(2, n_of_bits);                          // Get the number of step
	unsigned char step = 256 / n_of_steps;                       // Get step size
	unsigned int* R_Dlevels = new unsigned int[n_of_steps + 1];  // Array to save Decision levels
	unsigned int* G_Dlevels = new unsigned int[n_of_steps + 1];  //
	unsigned int* B_Dlevels = new unsigned int[n_of_steps + 1];  //
	unsigned char* R_Rlevels = new unsigned char[n_of_steps];    // Array to save Reconstruction levels
	unsigned char* G_Rlevels = new unsigned char[n_of_steps];    //
	unsigned char* B_Rlevels = new unsigned char[n_of_steps];    //
	unsigned char R_map[256], G_map[256], B_map[256];            // Mapping function

	R_Dlevels[0] = 0, R_Dlevels[n_of_steps] = 256;  // Save lower&upper bound of decision levels
	G_Dlevels[0] = 0, G_Dlevels[n_of_steps] = 256;  //
	B_Dlevels[0] = 0, B_Dlevels[n_of_steps] = 256;  //
	for (int i = 1; i < n_of_steps; i++)            // Initialize decision levels
	{											    //
		R_Dlevels[i] = R_Dlevels[i - 1] + step;     //
		G_Dlevels[i] = G_Dlevels[i - 1] + step;     //
		B_Dlevels[i] = B_Dlevels[i - 1] + step;     //
	}												//

	for (int k = 0; k < n_of_iter; k++)
	{			                             						   
		for (int i = 0; i < n_of_steps; i++)						   // Calculate reconstuction levels
		{															   // 
			int sum = 0, cnt = 0;									   // R data's reconstuction level
			for (int j = R_Dlevels[i]; j < R_Dlevels[i + 1]; j++)	   // 
			{														   // 
				sum += R_histogram[j] * j;				               // get sum of sample in this step
				cnt += R_histogram[j];				                   // get the number of sample in this step
			}				                                           //
			if (cnt == 0)		                          		       // if no sample in this step,
				R_Rlevels[i] = (R_Dlevels[i] + R_Dlevels[i + 1]) / 2;  // reconstruction level = middle of step
			else		                          		               // if sample exist in this step,
				R_Rlevels[i] = sum / cnt;		                       // reconstruction level = average of sample
				                            						   //
			sum = 0, cnt = 0;		                          		   // G data's reconstuction level
			for (int j = G_Dlevels[i]; j < G_Dlevels[i + 1]; j++)	   //
			{		                          		                   //
				sum += G_histogram[j] * j;		                       //
				cnt += G_histogram[j];		                           //
			}		                          		                   //
			if (cnt == 0)		                          		       //
				G_Rlevels[i] = (G_Dlevels[i] + G_Dlevels[i + 1]) / 2;  //
			else		                          		               //
				G_Rlevels[i] = sum / cnt;		                       //
				                            						   //
			sum = 0, cnt = 0;		                          		   // B data's reconstuction level
			for (int j = B_Dlevels[i]; j < B_Dlevels[i + 1]; j++)	   //
			{		                          		                   //
				sum += B_histogram[j] * j;		                       //
				cnt += B_histogram[j];		                           //
			}		                          		                   //
			if (cnt == 0)		                          		       //
				B_Rlevels[i] = (B_Dlevels[i] + B_Dlevels[i + 1]) / 2;  //
			else                                                       //
				B_Rlevels[i] = sum / cnt;                              //
		}
		for (int i = 1; i < n_of_steps; i++)                       // Calculate decision levels
		{                                                          //
			R_Dlevels[i] = (R_Rlevels[i - 1] + R_Rlevels[i]) / 2;  // D[i] = (R[i-1]+R[i])/2
			G_Dlevels[i] = (G_Rlevels[i - 1] + G_Rlevels[i]) / 2;  //
			B_Dlevels[i] = (B_Rlevels[i - 1] + B_Rlevels[i]) / 2;  //
		}
		for (int i = 0; i < n_of_steps; i++)                       // Get mapping function
		{                                                          //
			for (int j = R_Dlevels[i]; j < R_Dlevels[i + 1]; j++)  // 
				R_map[j] = i;                                      // 
			for (int j = G_Dlevels[i]; j < G_Dlevels[i + 1]; j++)  //
				G_map[j] = i;                                      //
			for (int j = B_Dlevels[i]; j < B_Dlevels[i + 1]; j++)  //
				B_map[j] = i;                                      //
		}
		// Code to calculate MSE
		double mse = 0;                                               // 
		for (int i = 0; i < 256; i++)                                 //
		{                                                             //
			mse += pow(i - R_Rlevels[R_map[i]], 2) * R_histogram[i];  // error in R 
			mse += pow(i - G_Rlevels[G_map[i]], 2) * G_histogram[i];  // error in G
			mse += pow(i - B_Rlevels[B_map[i]], 2) * B_histogram[i];  // error in B
		}                                                             //
		mse = mse / (3 * width * height);                             //
		cout << "iteration" << k+1 << ": " << mse << endl;            //

	}
	
	for (int i = 0; i < height; i++)                        // Map input value to n-bits code
	{                                                       //
		for (int j = 0; j < width; j++)                     //
		{                                                   //
			output_Rdata[i][j] = R_map[input_Rdata[i][j]];  // 
			output_Gdata[i][j] = G_map[input_Gdata[i][j]];  //
			output_Bdata[i][j] = B_map[input_Bdata[i][j]];  //
		}                                                   //
	}

	/* Encoding quantization code to output code file */
	Encoding(code_file, height, width, n_of_bits, output_Rdata, output_Gdata, output_Bdata);

	/* Save decision & reconstruction levels to codebook file */
	SaveCodebook(codebook_file, n_of_steps, R_Rlevels, G_Rlevels, B_Rlevels);

	for (int i = 0; i < height; i++) 
	{
		delete[] input_Rdata[i],  input_Gdata[i],  input_Bdata[i];
		delete[] output_Rdata[i], output_Gdata[i], output_Bdata[i];
	}
	delete[] input_Rdata,  input_Gdata,  input_Bdata;
	delete[] output_Rdata, output_Gdata, output_Bdata;
	delete[] R_Dlevels, G_Dlevels, B_Dlevels;
	delete[] R_Rlevels, G_Rlevels, B_Rlevels;
	return true;
}

/* Encoding quantization code to output code file */
void Encoding(string code_file,              /* output code file     */
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
				if (code_size >= remain_bit) {                // if buffer is full,                                                                
					code_size -= remain_bit;                  // update number of bits to write                                         
					RGB_code = RGB_code << remain_bit;        // update RGB code and align left    
					fputc(buf, fout);                         // write buffer to file    
					buf = 0;                                  // reset buffer     
					remain_bit = 8;                           //                            
				}                                             //                                         
				else {                                        // if buffer isn't full,     
					remain_bit -= code_size;                  // save buffer            
					code_size = 0;                            // and go next pixel          
				}                                             //   
			}                                                 //   
		}                                                     //
	}                                                         // 
	fputc(buf, fout);                                         // write last buffer
	fclose(fout);                                             // Close output code file
}

/* Save decision & reconstruction levels to codebook file */
void SaveCodebook(string codebook,           /* output codebook file       */
				  int n_of_step,             /* the number of stemp        */
				  unsigned char* R_Rlevels,  /* reconstruction levels of R */
				  unsigned char* G_Rlevels,  /* reconstruction levels of G */
				  unsigned char* B_Rlevels)  /* reconstruction levels of B */
{
	FILE* fout = fopen(codebook.c_str(), "w+b");             // Open output codebook file 
	if (fout == NULL) {                                      // If open fail, 
		cout << "Error: codebook file create fail" << endl;  // print error message
		exit(-1);                                            // and exit program
	}

	for (int i = 0; i < n_of_step; i++)  // Save reconstruction level 
	{                                    //
		fputc(R_Rlevels[i], fout);       //
		fputc(G_Rlevels[i], fout);       // 
		fputc(B_Rlevels[i], fout);       //
	}
}