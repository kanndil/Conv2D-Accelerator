#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
#define DSIZE       1024
#define image_width   62
#define image_height  12

#define kernel_width   3
#define kernel_height  3

#define stride_x  1
#define stride_y  1

#define result_width   ((image_width - kernel_width) / stride_x + 1)
#define result_height  ((image_height - kernel_height) / stride_y + 1)


int image[DSIZE];
int kernel[9] = {1, 0, 1, 1, 0, 1, 1, 0, 1};
int result[result_width * result_height];

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

    for (int i = 0; i < result_height; i++) {
        for (int j = 0; j < result_width; j++) {
            int sum = 0;
            for (int k_i = 0; k_i < kernel_height; k_i++) {
                for (int k_j = 0; k_j < kernel_width; k_j++) {
                    int image_i = i * stride_y + k_i;
                    int image_j = j * stride_x + k_j;
                    sum += image[image_i * image_width + image_j] * kernel[k_i * kernel_width + k_j];
                }
            }
            // Saturate the sum to be within -128 to 127
            if (sum > 127) {
                sum = 127;
            } else if (sum < -128) {
                sum = -128;
            }
            result[i * result_width + j] = sum;
        }
    }

    // Print the 2D result
    for (int i = 0; i < result_height; i++) {
        for (int j = 0; j < result_width; j++) {
            printf("%d ", result[i * result_width + j]);
        }
        printf("\n");
    }

    // Write the result to a text file
    FILE *file = fopen("golden_model_output.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    for (int i = 0; i < result_height; i++) {
        for (int j = 0; j < result_width; j++) {
            fprintf(file, "%d ", result[i * result_width + j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);

    return 0;
}
