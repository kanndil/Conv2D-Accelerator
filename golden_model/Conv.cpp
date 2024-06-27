#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
#define DSIZE       1024
#define IMAGE_WIDTH  62
#define IMAGE_HEIGHT  12

#define KERNEL_3x3 1
//#define KERNEL_5x5 1


#ifdef KERNEL_3x3
        #define KERNEL_WIDTH  3
        #define KERNEL_HEIGHT  3
        int kernel[9] = {   1, 0, 1, 
                            1, 0, 1, 
                            1, 0, 1};
#endif

#ifdef KERNEL_5x5
        #define KERNEL_WIDTH  5
        #define KERNEL_HEIGHT  5
        int kernel[25] = {  1, 0, 1, 0, 1,
                            1, 0, 1, 0, 1,
                            1, 0, 1, 0, 1, 
                            1, 0, 1, 0, 1, 
                            1, 0, 1, 0, 1};
#endif


#define STRIDE_X  1
#define STRIDE_Y  1

#define RESULT_WIDTH  ((IMAGE_WIDTH - KERNEL_WIDTH) / STRIDE_X + 1)
#define RESULT_HEIGHT ((IMAGE_HEIGHT - KERNEL_HEIGHT) / STRIDE_Y + 1)

int image[DSIZE];
int result[RESULT_WIDTH * RESULT_HEIGHT];


void read_input_image(string input_file) {
  ifstream infile(input_file);

    if (infile.is_open()) {
    for (int i = 0; i < DSIZE; ++i) {
        infile >> image[i]; // Read each number from the file into the array
    }
    infile.close(); // Close the file once done reading
    } else {
        cout << "Unable to open file." << endl;
    } 
}

int main() {
    read_input_image("../src/input_image.txt");

    for (int i = 0; i < RESULT_HEIGHT; i++) {
        for (int j = 0; j < RESULT_WIDTH; j++) {
            int sum = 0;
            for (int k_i = 0; k_i < KERNEL_HEIGHT; k_i++) {
                for (int k_j = 0; k_j < KERNEL_WIDTH; k_j++) {
                    int image_i = i * STRIDE_Y + k_i;
                    int image_j = j * STRIDE_X + k_j;
                    sum += image[image_i * IMAGE_WIDTH + image_j] * kernel[k_i * KERNEL_WIDTH + k_j];
                }
            }
            // Saturate the sum to be within -128 to 127
            if (sum > 127) {
                sum = 127;
            } else if (sum < -128) {
                sum = -128;
            }
            result[i * RESULT_WIDTH + j] = sum;
        }
    }

    // Print the 2D result
    for (int i = 0; i < RESULT_HEIGHT; i++) {
        for (int j = 0; j < RESULT_WIDTH; j++) {
            printf("%d ", result[i * RESULT_WIDTH + j]);
        }
        printf("\n");
    }

    // Write the result to a text file
    FILE *file = fopen("golden_model_output.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    for (int i = 0; i < RESULT_HEIGHT; i++) {
        for (int j = 0; j < RESULT_WIDTH; j++) {
            fprintf(file, "%d ", result[i * RESULT_WIDTH + j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);

    return 0;
}
