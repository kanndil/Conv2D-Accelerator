#include "One_Channel_Conv.h"

#ifdef KERNEL_3x3
    q7_t kernel[9] = {   1, 0, 1, 
                        1, 0, 1, 
                        1, 0, 1};
#endif
#ifdef KERNEL_5x5
    q7_t kernel[25] = {  1, 0, 1, 0, 1,
                        1, 0, 1, 0, 1,
                        1, 0, 1, 0, 1, 
                        1, 0, 1, 0, 1, 
                        1, 0, 1, 0, 1};
#endif

q7_t image[DSIZE];
q7_t result[RESULT_WIDTH * RESULT_HEIGHT];

using namespace std;

void read_input_image(string input_file);
void write_output_image();
void print_result();
void one_channel_conv(  q7_t *input_image,                         
                        q7_t *result, 
                        q7_t *input_kernel,
                        q31_t bias,                                 // bias
                        const uint16_t dim_im_in_x,                // input image dimention x
	                    const uint16_t dim_im_in_y,                // input image dimention y
                        const uint16_t dim_kernel_x,               // filter kernel size x
	                    const uint16_t dim_kernel_y,               // filter kernel size y
                        const uint16_t stride_x,                   // stride x
	                    const uint16_t stride_y,                   // stride y
                        const uint16_t dim_im_out_x,               // output image dimension x
	                    const uint16_t dim_im_out_y,               // output image dimension y
                        const uint16_t output_shift               // output shift
);

int main() {
    read_input_image("../src/input_image.txt");

    one_channel_conv(image, result, kernel, 0, IMAGE_WIDTH, IMAGE_HEIGHT, KERNEL_WIDTH, KERNEL_HEIGHT, STRIDE_X, STRIDE_Y, RESULT_WIDTH, RESULT_HEIGHT, 0);

    print_result();

    // Write the result to a text file
    write_output_image();

    return 0;
}

void one_channel_conv(  q7_t *input_image,                         
                        q7_t *result, 
                        q7_t *input_kernel,
                        q31_t bias,                                 // bias
                        const uint16_t dim_im_in_x,                // input image dimention x
	                    const uint16_t dim_im_in_y,                // input image dimention y
                        const uint16_t dim_kernel_x,               // filter kernel size x
	                    const uint16_t dim_kernel_y,               // filter kernel size y
                        const uint16_t stride_x,                   // stride x
	                    const uint16_t stride_y,                   // stride y
                        const uint16_t dim_im_out_x,               // output image dimension x
	                    const uint16_t dim_im_out_y,               // output image dimension y
                        const uint16_t output_shift                // output shift
)
{

    for (int i = 0; i < dim_im_out_y; i++) {
        for (int j = 0; j < dim_im_out_x; j++) {
            int sum = bias;
            for (int k_i = 0; k_i < dim_kernel_y; k_i++) {
                for (int k_j = 0; k_j < dim_kernel_x; k_j++) {
                    int image_i = i * stride_y + k_i;
                    int image_j = j * stride_x + k_j;
                    sum += input_image[image_i * dim_im_in_x + image_j] * input_kernel[k_i * dim_kernel_x + k_j];
                }
            }
            // Saturate the sum to be within -128 to 127
            result[i * dim_im_out_x + j] = (q7_t)__NNOM_SSAT((sum >> output_shift), 8);
        }
    }
}


void read_input_image(string input_file) {
  ifstream infile(input_file);

    if (infile.is_open()) {
    for (int i = 0; i < DSIZE; ++i) {
        int temp=0;
        infile >> temp;
        image[i]= static_cast<int8_t>(temp); // Read each number from the file into the array
    }
    infile.close(); // Close the file once done reading
    } else {
        cout << "Unable to open file." << endl;
    } 
}

void write_output_image() {
    FILE *file = fopen("golden_model_output.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return ;
    }
    for (int i = 0; i < RESULT_HEIGHT; i++) {
        for (int j = 0; j < RESULT_WIDTH; j++) {
            fprintf(file, "%d ", result[i * RESULT_WIDTH + j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void print_result() {
    // Print the 2D result
    for (int i = 0; i < RESULT_HEIGHT; i++) {
        for (int j = 0; j < RESULT_WIDTH; j++) {
            printf("%d ", result[i * RESULT_WIDTH + j]);
        }
        printf("\n");
    }
}