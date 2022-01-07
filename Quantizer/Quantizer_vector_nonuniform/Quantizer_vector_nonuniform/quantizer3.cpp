#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


bool Vector_nonuniform_quantization(string, string, string, int, int, int, int);
void Encoding(string, int, int, int, unsigned char*);
void SaveCodebook(string, int, unsigned char**);


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
	/* set K-means algorithm's iteration */
	int n_of_iter = 10;

	if (Vector_nonuniform_quantization(input_file, code_file, codebook_file, input_width, input_height, n_of_bits, n_of_iter))
		cout << "Quantization success" << endl;
	else
		cout << "Quantization fail" << endl;

	return 0;
}

/* Vector non-uniform quantize for input file and create codebook & encoded code file */
bool Vector_nonuniform_quantization(string intput_file,    /* input image file        */
									string code_file,      /* output code file        */
									string codebook_file,  /* output code file        */
									int width,             /* input image's width     */
									int height,            /* input image's height    */
									int n_of_bits,         /* n-bits of code          */
									int n_of_iter)         /* the number of iteration */
{
	int n_of_steps = pow(2, n_of_bits);  // Get the number of steps

	unsigned char** RGB_vector = new unsigned char* [height * width];  // Array to save input RGB data
	for (int i = 0; i < height * width; i++)                           //
		RGB_vector[i] = new unsigned char[3];                          //

	unsigned char* RGB_code = new unsigned char[height * width];  // Array to save output RGB code (group)

	unsigned char** codebook = new unsigned char* [n_of_steps];  // Array to save codebook
	for (int i = 0; i < n_of_steps; i++)                         //
		codebook[i] = new unsigned char[3];                      //

	unsigned int** group_sum = new unsigned int* [n_of_steps];  // Array to save samples's sum of each group
	for (int i = 0; i < n_of_steps; i++)                        //
		group_sum[i] = new unsigned int[3];                     //
	unsigned int* group_count = new unsigned int[n_of_steps];   // Array to save the number of samples of each group

	/* Read input RGB data to vector */
	FILE* fin = fopen(intput_file.c_str(), "rb");       // Open input image file
	if (fin == NULL) {                                  // If open fail,
		cout << "Error: Input file open fail" << endl;  // print error message
		exit(-1);                                       // and exit program
	}                                                   //
	for (int i = 0; i < height * width; i++)            // Read input raw file data to vector
	{                                                   //
		RGB_vector[i][0] = fgetc(fin);                  //
		RGB_vector[i][1] = fgetc(fin);                  //
		RGB_vector[i][2] = fgetc(fin);                  //
	}                                                   //
	fclose(fin);                                        // Close input file stream

	/* Initialize reconstructed vector */
	int idx = 0;                                                 // 
	int Rd = n_of_bits / 3;                                      // To distribute the groups fairly,
	int Gd = (n_of_bits - Rd) / 2;                               // divide the number of code into three
	int Bd = n_of_bits - Rd - Gd;                                // and set R, G, B axis with that 
	Rd = pow(2, Rd), Gd = pow(2, Gd), Bd = pow(2, Bd);           //
	for (int z = 0; z < Bd; z++)                                 // B axis
	{                                                            //
		for (int y = 0; y < Gd; y++)                             // G axis
		{                                                        //
			for (int x = 0; x < Rd; x++)                         // R axis 
			{                                                    //
				codebook[idx][0] = (256 / Rd) * x + (128 / Rd);  // set initial reconstructed vector
				codebook[idx][1] = (256 / Gd) * y + (128 / Gd);  //
				codebook[idx][2] = (256 / Bd) * z + (128 / Bd);  //
				idx++;                                           //
			}                                                    //
		}                                                        //
	}                                                            //

	for (int it = 0; it < n_of_iter; it++)
	{  
		for (int i = 0; i < n_of_steps; i++)  // 
		{                                     // 
			group_sum[i][0] = 0;              // Reset samples's sum of each group
			group_sum[i][1] = 0;              // 
			group_sum[i][2] = 0;              // 
			group_count[i] = 0;               // Reset the number of samples of each group
		}                                     // 
		
		/* Classify input vectors into groups */
		for (int i = 0; i < height * width; i++)                      // Loop all input vectors
		{                                                             //
			unsigned int min_dist = UINT_MAX;                         // 
			for (int j = 0; j < n_of_steps; j++)                      // Loop all groups
			{                                                         //
				unsigned int dist = 0;                                //
				dist += pow((codebook[j][0] - RGB_vector[i][0]), 2);  // Calculate distance between 
				dist += pow((codebook[j][1] - RGB_vector[i][1]), 2);  // input vector and group's center
				dist += pow((codebook[j][2] - RGB_vector[i][2]), 2);  // 
				if (min_dist > dist) {                                // If find new closest group,
					min_dist = dist;                                  // update new closest distance 
					RGB_code[i] = j;                                  // and classify input vector into that group
				}                                                     //
			}                                                         //
													 			      //
			group_sum[RGB_code[i]][0] += RGB_vector[i][0];            // Sum samples of each group
			group_sum[RGB_code[i]][1] += RGB_vector[i][1];            //
			group_sum[RGB_code[i]][2] += RGB_vector[i][2];            //
			group_count[RGB_code[i]]++;                               // Count samples of each group
		}                                                             //                                

		/* Recalculate reconstructed vector(group's center) */
		int variation = 0;                                                   // 
		for (int i = 0; i < n_of_steps; i++)                                 // Loop all groups
		{                                                                    // 
			if (group_count [i]!= 0) {                                       // If this group have samples,
				unsigned char prev_codebook[3];                              // 
				for (int j = 0; j < 3; j++)                                  // 
				{                                                            //
					prev_codebook[j] = codebook[i][j];                       // save prev center
					codebook[i][j] = group_sum[i][j] / group_count[i];       // set sample average as new center
					variation += pow(prev_codebook[j] - codebook[i][j], 2);  // calculate variation
				}                                                            //
			}                                                                //
		}                                                                    //
		cout << "Iter" << it+1 << "'s variation : " << variation << endl;    // Print total variation
	}

	/* Encoding quantization code to output code file */
	Encoding(code_file, height, width, n_of_bits, RGB_code);

	/* Save decision & reconstruction levels to codebook file */
	SaveCodebook(codebook_file, n_of_steps, codebook);

	for (int i = 0; i < height * width; i++)
		RGB_vector[i];
	for (int i = 0; i < n_of_steps; i++)
	{
		codebook[i];
		group_sum[i];
	}
	delete[] RGB_vector, RGB_code;
	delete[] codebook, group_sum, group_count;
	return true;
}

/* Encoding quantizatized code to output code file */
void Encoding(string code_file,         /* output code file     */
			  int height,               /* input image's height */
			  int width,                /* input image's width  */
			  int n_of_bits,            /* n-bits of code       */
			  unsigned char* RGB_code)  /* quantized RGB code   */
{
	FILE* fout = fopen(code_file.c_str(), "w+b");        // Open output code file 
	if (fout == NULL) {                                  // If open fail, 
		cout << "Error: code file create fail" << endl;  // print error message
		exit(-1);                                        // and exit program
	}

	unsigned char buf = 0;                         // 8-bits buffer to write file
	int vaild_bits = 0;                            // the nubmer of vaild bits in buffer
	for (int i = 0; i < height * width; i++)       // 
	{                                              // 
		int empty_bits = 8 - vaild_bits;           // the nubmer of empty bits in buffer
		if (empty_bits < n_of_bits) {              // If not enough space in buffer,
			unsigned char temp = RGB_code[i];      // 
			temp = temp << (8 - n_of_bits);        // 
			buf = buf | (temp >> vaild_bits);      // fill buffer with part of code
			temp = temp << (empty_bits);           // 
		    fputc(buf, fout);                      // write buffer to file
			buf = 0 | temp;                        // reset and fill buffer with remain code
			vaild_bits = n_of_bits - empty_bits;   // count valid bits
		}                                          // 
		else {                                     // If enough space in buffer,
			unsigned char temp = RGB_code[i];      // 
			temp = temp << (8 - n_of_bits);        // 
			buf = buf | (temp >> vaild_bits);      // fill buffer with code
			vaild_bits += n_of_bits;               // count valid bits
		}                                          // 
	}                                              // 
	fputc(buf, fout);                              // write last buffer to file
	fclose(fout);                                  // Close output code file
}

/* Save codeword & reconstruction vector pair to codebook file */
void SaveCodebook(string codebook_file,      /* output codebook file                  */
				  int n_of_step,             /* the number of stemp                   */
				  unsigned char** codebook)  /* codeword & reconstruction vector pair */
{
	FILE* fout = fopen(codebook_file.c_str(), "w+b");        // Open output codebook file 
	if (fout == NULL) {                                      // If open fail, 
		cout << "Error: codebook file create fail" << endl;  // print error message
		exit(-1);                                            // and exit program
	}

	for (int i = 0; i < n_of_step; i++)   // Save reconstruction vector 
	{                                     //
		for (int j = 0; j < 3; j++)       //
			fputc(codebook[i][j], fout);  //
	}
}