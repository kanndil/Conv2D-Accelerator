
#include "One_Channel_Conv.h"
#define nnom_qformat_param_t int32_t // this should match the backend, need a better way to do it. 
#define nnom_shape_data_t uint16_t
#define NNOM_ROUND(out_shift) ((0x1 << out_shift) >> 1 )
// tensor quantisation types
typedef enum
{
	NNOM_QTYPE_PER_TENSOR = 0,
	NNOM_QTYPE_PER_AXIS = 1
} nnom_qtype_t;

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))



void print_output_image(int *output_image, int outdim_x, int outdim_y, int output_channels) {
    for (int k = 0; k < output_channels; k++) {
        for (int i = 0; i < outdim_x; i++) {
            for (int j = 0; j < outdim_y; j++) {
                printf("%d ", output_image[i + (j * outdim_x + k) * output_channels]);
            }
        printf("\n");
        }
    printf("\n\n\n\n");
    }
}

void local_convolve_HWC_q7_nonsquare_1(const q7_t *Im_in,              // input image
	const uint16_t dim_im_in_x,                                        // input image dimention x
	const uint16_t dim_im_in_y,                                        // input image dimention y
	const uint16_t ch_im_in,                                           // number of input image channels
	const q7_t *wt,                                                    // kernel weights
	const uint16_t ch_im_out,                                          // number of filters, i.e., output image channels
	const uint16_t dim_kernel_x,                                       // filter kernel size x
	const uint16_t dim_kernel_y,                                       // filter kernel size y
	const uint16_t padding_x,                                          // padding sizes x
	const uint16_t padding_y,                                          // padding sizes y
	const uint16_t stride_x,                                           // stride x
	const uint16_t stride_y,                                           // stride y
    const uint16_t dilation_x,                                         // dilation x
	const uint16_t dilation_y,                                         // dilation y
	const q7_t *bias,                                                  // bias
	const nnom_qformat_param_t *bias_shift,                            // bias shifts
    const nnom_qformat_param_t *out_shift,                             // output shift
    const nnom_qtype_t q_type,                                         // per channel or per tensor
    int *Im_out,                                                       // output image
	const uint16_t dim_im_out_x,                                       // output image dimension x
	const uint16_t dim_im_out_y,                                       // output image dimension y
	q15_t *bufferA,                                                    // buffer space for input
	q7_t *bufferB                                                      // buffer space for output
)
{  
    int i, j, k, l, m, n;
    int conv_out;
    int in_row, in_col;
    int in_pix_loc, wt_loc;
    int shift_idx, shift_steps;

        shift_steps = 0;
    for (i = 0, shift_idx = 0; i < ch_im_out; i++, shift_idx += shift_steps)
    {
        int temp_bias = 0;
        int outshift = out_shift[shift_idx];
        if(bias)
            temp_bias = ((q31_t)(bias[i]) << bias_shift[shift_idx]) + NNOM_ROUND(out_shift[shift_idx]);
        else
            temp_bias = (q31_t) NNOM_ROUND(out_shift[shift_idx]);
            

        for (j = 0; j < dim_im_out_y; j++)
        {
            int32_t base_idx_y = stride_y * j - padding_y;
            for (k = 0; k < dim_im_out_x; k++)
            {
				int32_t base_idx_x = stride_x * k - padding_x;
                int32_t ker_y_start = MAX(0, -(base_idx_y-(dilation_y-1))/dilation_y);
                int32_t ker_x_start = MAX(0, -(base_idx_x-(dilation_x-1))/dilation_x);
                int32_t ker_y_end = MIN(dim_kernel_y, (dim_im_in_y - base_idx_y + (dilation_y-1))/dilation_y);
                int32_t ker_x_end = MIN(dim_kernel_x, (dim_im_in_x - base_idx_x + (dilation_x-1))/dilation_x);

                conv_out = temp_bias;

                for (m = ker_y_start; m < ker_y_end; m++)
                {
                    for (n = ker_x_start; n < ker_x_end; n++)
                    {
                        in_row = stride_y * j + m * (dilation_y - padding_y);
                        in_col = stride_x * k + n * dilation_x - padding_x;

                        // pre-calculate the pixel location and weight location to improve the performance.
                        in_pix_loc = (in_row * dim_im_in_x + in_col) * ch_im_in;
                        wt_loc = i * ch_im_in * dim_kernel_y * dim_kernel_x + (m * dim_kernel_x + n) * ch_im_in;

                        for (l = 0; l < ch_im_in; l++)
                        {     
                            conv_out += Im_in[in_pix_loc + l] * wt[wt_loc + l];
                        } 
                    }
                }
                Im_out[i + (j * dim_im_out_x + k) * ch_im_out] = (q7_t)__NNOM_SSAT((conv_out >> outshift), 8);
            }
        }
    }
}



void one_channel_conv(  q7_t *input_image,                         
                        q31_t *result,
                        q7_t *input_kernel,
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
            int sum = 0;
            for (int k_i = 0; k_i < dim_kernel_y; k_i++) {
                for (int k_j = 0; k_j < dim_kernel_x; k_j++) {
                    int image_i = i * stride_y + k_i;
                    int image_j = j * stride_x + k_j;
                    sum += input_image[image_i * dim_im_in_x + image_j] * input_kernel[k_i * dim_kernel_x + k_j];
                }
            }
            result[i * dim_im_out_x + j] = sum;
        }
    }
}


void local_convolve_HWC_q7_nonsquare_2(const q7_t *Im_in,              // input image
	const uint16_t dim_im_in_x,                                        // input image dimention x
	const uint16_t dim_im_in_y,                                        // input image dimention y
	const uint16_t ch_im_in,                                           // number of input image channels
	const q7_t *wt,                                                    // kernel weights
	const uint16_t ch_im_out,                                          // number of filters, i.e., output image channels
	const uint16_t dim_kernel_x,                                       // filter kernel size x
	const uint16_t dim_kernel_y,                                       // filter kernel size y
	const uint16_t padding_x,                                          // padding sizes x
	const uint16_t padding_y,                                          // padding sizes y
	const uint16_t stride_x,                                           // stride x
	const uint16_t stride_y,                                           // stride y
    const uint16_t dilation_x,                                         // dilation x
	const uint16_t dilation_y,                                         // dilation y
	const q7_t *bias,                                                  // bias
	const nnom_qformat_param_t *bias_shift,                            // bias shifts
    const nnom_qformat_param_t *out_shift,                             // output shift
    const nnom_qtype_t q_type,                                         // per channel or per tensor
    int *Im_out,                                                       // output image
	const uint16_t dim_im_out_x,                                       // output image dimension x
	const uint16_t dim_im_out_y,                                       // output image dimension y
	q15_t *bufferA,                                                    // buffer space for input
	q7_t *bufferB                                                      // buffer space for output
)
{
    
    q7_t Image_channels[ch_im_in][dim_im_in_y*dim_im_in_x];
    q7_t Kernel_filters [ch_im_out][ch_im_in][dim_kernel_y*dim_kernel_x ];
    
    // Copy the input image into the 2d array
    for (int i = 0; i < dim_im_in_y; i++)  
        for (int j = 0; j < dim_im_in_x; j++)  
            for (int k = 0; k < ch_im_in; k++) 
                Image_channels[k][i*dim_im_in_x + j] = Im_in[(i*dim_im_in_x + j)*ch_im_in + k];

    
    // Copy the kernel into the 3d array
    for (int i = 0; i < ch_im_out; i++) 
        for (int j = 0; j < ch_im_in; j++) 
            for (int k = 0; k < dim_kernel_y; k++) 
                for (int l = 0; l < dim_kernel_x; l++) 
                    Kernel_filters[i][j][k*dim_kernel_x + l] = wt[i * ch_im_in * dim_kernel_y * dim_kernel_x + (k*dim_kernel_x + l)*ch_im_in+j]; // Example initialization
 
    
    // Im_in:           [image height][image width][input channels]
    // kernal weights:  [kernel filters][kernel height][kernel width][input channels]
    
    q31_t result[dim_im_out_y*dim_im_out_x];
    q31_t resultchannels[dim_im_out_y*dim_im_out_x*ch_im_out];

    int i, j, k, l, m, n;
    int conv_out;
    int in_row, in_col;
    int in_pix_loc, wt_loc;
    int shift_idx, shift_steps;
    if(q_type == NNOM_QTYPE_PER_AXIS)
        shift_steps = 1;
    else
        shift_steps = 0;
    
    for (i = 0, shift_idx = 0; i < ch_im_out; i++, shift_idx += shift_steps)
        for (k = 0; k < dim_im_out_y; k++)
            for (l = 0; l < dim_im_out_x; l++)
                resultchannels[i + (k * dim_im_out_x + l) * ch_im_out] = 0;
    
    
    for (i = 0, shift_idx = 0; i < ch_im_out; i++, shift_idx += shift_steps)
    {
        int temp_bias = 0;
        int outshift = out_shift[shift_idx];
        if(bias)
            temp_bias = ((q31_t)(bias[i]) << bias_shift[shift_idx]) + NNOM_ROUND(out_shift[shift_idx]);
        else
            temp_bias = (q31_t) NNOM_ROUND(out_shift[shift_idx]);
        
        for (j = 0; j < ch_im_in; j++)
        {
            one_channel_conv(   Image_channels[j], result, Kernel_filters[i][j],
                                dim_im_in_x, dim_im_in_y, dim_kernel_x, dim_kernel_y,
                                stride_x, stride_y,dim_im_out_x, dim_im_out_y, outshift );
            
            for (k = 0; k < dim_im_out_y; k++)
                for (l = 0; l < dim_im_out_x; l++)
                    resultchannels[i + (k * dim_im_out_x + l) * ch_im_out] += result[k * dim_im_out_x + l];
            
            if (j==(ch_im_in-1)){
                for (k = 0; k < dim_im_out_y; k++){
                    for (l = 0; l < dim_im_out_x; l++){
                        int temp=resultchannels[i + (k * dim_im_out_x + l) * ch_im_out] + temp_bias ;
                        Im_out[i + (k * dim_im_out_x + l) * ch_im_out]= (q7_t)__NNOM_SSAT((temp >> outshift), 8);
                    }
                }     
            } 
        }   
    }
}


int main () {

    int input_channels =3,output_channels = 2;
    int outdim_x = RESULT_WIDTH, outdim_y = RESULT_HEIGHT;

    q7_t bias [output_channels];
    int bias_shift[output_channels], out_shift[output_channels];

    q7_t input_image [IMAGE_HEIGHT*IMAGE_WIDTH*input_channels]; 
    q7_t kernels [KERNEL_HEIGHT*KERNEL_WIDTH*input_channels*output_channels];
    int output_image [outdim_x*outdim_y*output_channels];


    for (int i = 0; i < output_channels; i++) {
        bias[i] = 0; bias_shift[i] = 0; out_shift[i] = 1;
    }

    for (int i = 0; i < IMAGE_HEIGHT*IMAGE_WIDTH*input_channels; i++) 
        input_image[i] =  rand() % 10;
    
    for (int i = 0; i < KERNEL_HEIGHT*KERNEL_WIDTH*input_channels*output_channels; i++) 
        kernels[i] = rand() % 2;

    local_convolve_HWC_q7_nonsquare_1(  input_image, IMAGE_WIDTH, IMAGE_HEIGHT, input_channels, 
                                        kernels, output_channels, KERNEL_WIDTH, KERNEL_HEIGHT, 
                                        0, 0, STRIDE_X, STRIDE_Y, 1, 1, bias, bias_shift, out_shift, 
                                        NNOM_QTYPE_PER_TENSOR, output_image, outdim_x, outdim_y, NULL, NULL);

    print_output_image(output_image, outdim_x, outdim_y, output_channels);

    local_convolve_HWC_q7_nonsquare_2(  input_image, IMAGE_WIDTH, IMAGE_HEIGHT, input_channels, 
                                        kernels, output_channels, KERNEL_WIDTH, KERNEL_HEIGHT, 
                                        0, 0, STRIDE_X, STRIDE_Y, 1, 1, bias, bias_shift, out_shift, 
                                        NNOM_QTYPE_PER_TENSOR, output_image, outdim_x, outdim_y, NULL, NULL);

    print_output_image(output_image, outdim_x, outdim_y, output_channels);

    return 0;
}

