
#include <iostream>
#include <math.h>
#include "CL/opencl.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "clKernelSet.hpp"
#define AOCX_FILE "dfa.aocx"
#include <stdio.h>
#include <string.h>

#include <time.h>

const int LENGTH_OF_TEXT = 20240; 
const int LENGTH_OF_PATTERN = 50;

#define uchar unsigned char

void init_platform();
void freeResources();
void run();

// global variables
// ACL runtime configuration
static cl_platform_id platform;
static cl_device_id device;
static cl_context context;
static cl_program program;
static cl_int status;
static cl_command_queue queue;

static cl_mem text_buf, pattern_buf;

static char *text_c,*pattern_c;

int main( int argc, char** argv )
{
	srand (time(NULL));
	init_platform();
	run();
	freeResources();
	return 0;
}


template <typename T >
cl_mem alloc_shared_buffer (size_t size, T **vptr) {
  cl_mem res = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, sizeof(T) * size, NULL, &status);
  if(status != CL_SUCCESS) {
    printf("Failed clCreateBuffer. %d", status);
    freeResources();
    exit (1);
  }
  assert (vptr != NULL);
  *vptr = (T*)clEnqueueMapBuffer(queue, res, CL_TRUE, CL_MAP_WRITE|CL_MAP_READ, 0, sizeof(T) * size, 0, NULL, NULL, NULL);
  assert (*vptr != NULL);
  // Populate the buffer with garbage data
  for (size_t i=0; i< size; i++) {
    *((uchar*)(*vptr) + i) = 13;
  }
  clEnqueueUnmapMemObject(queue, res, *vptr, 0, NULL, NULL);
  return res;
}


void init_string( char* text, char* pattern )
{
	char alpha[] = { "abcdefghijklmnopqwxyz "};
	int length_of_alpha = strlen( alpha );
	for( int idx = 0 ; idx < LENGTH_OF_TEXT - 1 ; idx ++ ) {
		text_c [idx] = alpha[ rand() % length_of_alpha ];	
	}
	text[ LENGTH_OF_TEXT - 1 ] = 0;
	for( int idx = 0 ; idx < LENGTH_OF_PATTERN- 1; idx ++ ) {
		pattern_c [idx] = alpha[ rand() % (length_of_alpha-1) ];	
	}
	pattern[ LENGTH_OF_PATTERN-1 ] = 0 ;

// create a :match case
	text[49] = ' ';
	for( int idx = 0 ; idx < LENGTH_OF_PATTERN-1; idx ++ ){
		text[ 50 + idx ] = pattern[ idx ] ;
	}
	text[ 50 + LENGTH_OF_PATTERN - 1] = ' ';

	printf(" pattern = %s, text = %s\n", pattern, text);
}

void run() {
	time_t start_t, end_t;
	double diff_t;

	printf("Allocating buffers\n");

	text_buf = alloc_shared_buffer<char> (LENGTH_OF_TEXT, &text_c);
	pattern_buf= alloc_shared_buffer<char> (LENGTH_OF_PATTERN, &pattern_c);

	init_string( text_c, pattern_c );

	printf("Initializing kernels\n");
	size_t task_dim = 1;
	clKernelSet kernel_set (device, context, program);
	kernel_set.addKernel ("text_processor", 1, &task_dim, text_buf, LENGTH_OF_TEXT);
	kernel_set.addKernel ("word_processor", 1, &task_dim);
	kernel_set.addKernel ("matching", 1, &task_dim, pattern_buf, LENGTH_OF_PATTERN);

	printf("Launching the kernel...\n");
	time(&start_t);
	kernel_set.launch();

#if 0  
  // Enqueue 'stop' kernel when user want to exit
  printf("Press any key to stop processing...\n");
  getchar();

  clKernelSet kernel_set_2 (device, context, program);
  
  kernel_set_2.addKernel ("stop", 1, &task_dim, fps_buf);
  kernel_set_2.launch();
//  kernel_set_2.finish();

  printf("Stop kernel finished\n");
#endif 
  printf(" start waiting.... \n");
	kernel_set.finish();
//  kernel_set.waitEvent(2);  // watigin for mathcing
  time(&end_t);
  diff_t = difftime(end_t, start_t);
  printf(" done Execution time = %f\n", diff_t);


  return;
}

void init_platform() {

  cl_uint num_platforms;
  cl_uint num_devices;

  // Get the platform ID
  status = clGetPlatformIDs(1, &platform, &num_platforms);
  if(status != CL_SUCCESS) {
    printf("Failed clGetPlatformIDs. %d", status);
    freeResources();
    exit (1);
  }
  if(num_platforms != 1) {
    printf("Found %d platforms!\n", num_platforms);
    freeResources();
    exit (1);
  }

  // Get the device ID
  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &num_devices);
  if(status != CL_SUCCESS) {
    printf("Failed clGetDeviceIDs. %d", status);
    freeResources();
    exit (1);
  }
  if(num_devices != 1) {
    printf("Found %d devices!\n", num_devices);
    freeResources();
    exit (1);
  }

  // Create a context
  context = clCreateContext(0, 1, &device, NULL, NULL, &status);
  if(status != CL_SUCCESS) {
    printf("Failed clCreateContext. %d", status);
    freeResources();
    exit (1);
  }
  queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
  if(status != CL_SUCCESS) {
    printf("Failed to create queue. Error %d", status);
    freeResources();
    exit (1);
  }
  
  // Create the program using binary already compiled offline using aoc (i.e. the .aocx file)
  FILE* fp = fopen(AOCX_FILE, "rb");
  if (fp == NULL) {
    printf("Failed to open %s file (fopen).\n", AOCX_FILE);
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t binary_length = ftell(fp);
  unsigned char*binary = (unsigned char*) malloc(sizeof(unsigned char) * binary_length);
  assert(binary && "Malloc failed");
  rewind(fp);
  if (fread((void*)binary, binary_length, 1, fp) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
    exit (1);
  }
  fclose(fp);
  cl_int kernel_status;
  program = clCreateProgramWithBinary(context, 1, &device, &binary_length, (const unsigned char**)&binary, &kernel_status, &status);
  if(status != CL_SUCCESS || kernel_status != CL_SUCCESS) {
    printf("Failed clCreateProgramWithBinary. %d", status);
    freeResources();
    exit (1);
  }

  // Build the program
  status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
  if(status != CL_SUCCESS) {
    printf("Failed clBuildProgram. %d", status);
    freeResources();
    exit (1);
  }
}


// Free the resources allocated during initialization
void freeResources() {
  clReleaseProgram(program);
  clReleaseContext(context);
}
