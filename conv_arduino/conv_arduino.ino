#define DSIZE       1024
#define IMAGE_WIDTH  62
#define IMAGE_HEIGHT  12

// #define KERNEL_3x3 1
#define KERNEL_5x5 1


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

long elapsedTime = 0;
long loop_counter = 0;
long average_time = 0;

void setup() {
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // Initialize the image array
  for (int i = 0; i < DSIZE; i++) {
    image[i] = i;
  }
}

void loop() {
  // Record the start time
  long startTime = micros();

  // Perform the convolution
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

  // Calculate elapsed time
  long endTime = micros();
  elapsedTime += endTime - startTime;
  loop_counter++;

  if (loop_counter % 200 == 0) {
    average_time = elapsedTime / loop_counter;
    // Print the timing information
    Serial.print("Average Time: ");
    Serial.println(average_time);
  }

  // Add a delay to avoid flooding the serial monitor
  delay(10);
}
