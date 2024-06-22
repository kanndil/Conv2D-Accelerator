
#include <stdio.h>
using namespace std;
int main(){
    int image_width = 8;
    int image_height = 8;
    int image[image_width*image_height];

    int kernel_width = 3;
    int kernel_height = 3;
    int kernel[9]= {1, 0,-1,5,0,-1,1,0,-1};

    int result_width = image_width - kernel_width + 1;
    int result_height = image_height - kernel_height + 1;
    int result[result_width*result_height];


    for (int i =0; i< image_width*image_height; i++){
        image[i] = i;
    }

// print 2d array

    for (int i =0; i< image_width; i++){
        for (int j =0; j< image_height; j++){
            printf("%d ", image[i * image_width + j]);
        }
        printf("\n");
    }
    printf("\n\n");

    for (int i =0; i< result_width; i++){
        for (int j =0; j< result_height; j++){
            int sum = 0;
            for (int k_i = 0; k_i < kernel_height; k_i++) {
                for (int k_j = 0; k_j < kernel_width; k_j++) {
                    int image_i = i + k_i;
                    int image_j = j + k_j;
                    sum += image[image_i * image_width + image_j] * kernel[k_i * kernel_width + k_j];
                }
            }
            result[i * result_width + j] = sum;
        } 
    }


// print 2d using
    for (int i =0; i< result_width; i++){
        for (int j =0; j< result_height; j++){
            printf("%d ", result[i * result_width + j]);
        }
        printf("\n");
    }

    return 0;
}
