
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
#include <sys/time.h>


#include "performance.h"

const int LENGTH_OF_TEXT = 202400; 
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

static cl_mem text_buf, pattern_buf, result_buf;

static char *text_c,*pattern_c;
static char *result_c;

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

void init_answers( char* text, char* pattern, int count )
{
	for( int idx = 0 ; idx < count ; idx ++ ) {
		int position = rand() % ( LENGTH_OF_TEXT-LENGTH_OF_PATTERN )  + 1;

		text[position - 1] = ' ';
		for( int idx = 0 ; idx < LENGTH_OF_PATTERN-1; idx ++ ){
			text[ position+ idx ] = pattern[ idx ] ;
		}
		text[ position+ LENGTH_OF_PATTERN - 1] = ' ';	
	}	
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
	init_answers( text, pattern, 10);
/* 
	text[49] = ' ';
	for( int idx = 0 ; idx < LENGTH_OF_PATTERN-1; idx ++ ){
		text[ 50 + idx ] = pattern[ idx ] ;
	}
	text[ 50 + LENGTH_OF_PATTERN - 1] = ' ';
*/

	printf(" pattern = %s, text = %s\n", pattern, text);
}

void test_in_cpu( char* text, char* pattern )
{
	Performance p;
	p.start();

	bool match = false;
	int count = 0;
	for( int n = 0 ; n < LENGTH_OF_TEXT - 1; n ++ ) { 
		int m = 0;
		if( n == 0 || text[n-1] == ' ' ) {
			for( ; m < LENGTH_OF_PATTERN - 1; m ++ ) {
				if( pattern[m] != text[n+m] )
					break;
			}
		}
		if( m == LENGTH_OF_PATTERN - 1 )
			count ++;
	}

	p.stop();

	printf(" done Execution (CPU) time = (%u,%u), result = %d\n", p.report_sec(), p.report_usec(),count);
}

void run() {
	Performance p;

	printf("Allocating buffers\n");

	text_buf = alloc_shared_buffer<char> (LENGTH_OF_TEXT, &text_c);
	pattern_buf= alloc_shared_buffer<char> (LENGTH_OF_PATTERN, &pattern_c);
	result_buf= alloc_shared_buffer<char> (1, &result_c);
	result_c[0] = 0;

	init_string( text_c, pattern_c );

	printf("Initializing kernels\n");
	size_t task_dim = 1;
	size_t task_dim2 = 4;
	clKernelSet kernel_set (device, context, program);
//	kernel_set.addKernel ("text_processor", 1, &task_dim, text_buf, LENGTH_OF_TEXT);
//	kernel_set.addKernel ("word_processor", 1, &task_dim);
	kernel_set.addKernel ("word_processor", 1, &task_dim, text_buf, LENGTH_OF_TEXT );
	kernel_set.addKernel ("compare_length_processing", 1, &task_dim2, LENGTH_OF_PATTERN );
	kernel_set.addKernel ("matching", 1, &task_dim2, pattern_buf, LENGTH_OF_PATTERN, result_buf);

	printf("Launching the kernel...\n");
	p.start();
	kernel_set.launch();

	printf(" start waiting.... \n");
	kernel_set.finish();


	p.stop();
	printf(" done Execution (OpenCL Channel) time = (%u,%u), result = %d\n", p.report_sec(), p.report_usec(),result_c[0]);

	test_in_cpu( text_c, pattern_c );

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
