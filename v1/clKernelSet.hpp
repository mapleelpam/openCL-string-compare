/* Manages set of kernels connected by channels
 * All kernels can be created, launched, and waited on
 * as a single group. */

#include <vector> 
#include <assert.h>
#include "CL/opencl.h"
  
class clKernelSet {
 
  typedef struct {
    const char *kernel_name;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_uint work_dim;
    cl_event event;
    size_t *global_dim;
  } kernel_desc;
  
  typedef std::vector<kernel_desc> kernel_desc_vec;
  typedef kernel_desc_vec::iterator kernel_iter;
  
public:

  clKernelSet(cl_device_id device, cl_context context, cl_program program) {
    m_device = device;
    m_context = context;
    m_program = program;
  }
  ~clKernelSet() {
    for (kernel_iter iter = m_kernels.begin(); iter != m_kernels.end(); ++iter) {
      clReleaseCommandQueue (iter->queue);
      clReleaseKernel(iter->kernel);
    }
  }
  
  // Get a command queue. Use if don't care which kernel's queue you get.
  cl_command_queue getFirstCommandQueue(void) {
    assert (!m_kernels.empty());
    return m_kernels[0].queue; 
  }
  // Get a command queue. Use if don't care which kernel's queue you get.
  cl_event getFirstEvent(void) {
    assert (!m_kernels.empty());
    return m_kernels[0].event; 
  }  
  cl_event*  getEventN(int num) {
    assert (!m_kernels.empty());
    return &m_kernels[num].event; 
  } 
  
  
  // Launch all kernels in the set
  cl_int launch (void) {
    cl_int status, status_out = CL_SUCCESS;
    for (kernel_iter iter = m_kernels.begin(); iter != m_kernels.end(); ++iter) {
      status = clEnqueueNDRangeKernel(iter->queue, iter->kernel, iter->work_dim, NULL, iter->global_dim, iter->global_dim, 0, NULL, &iter->event);
      if (status != CL_SUCCESS) {
        printf ("Failed to launch kernel %s. Error %d\n", iter->kernel_name, status);
        status_out = status;
      }
    }
    return status_out;
  }

 void waitEvent( int num) {
	
  	clWaitForEvents( 1,  & m_kernels[num].event );
 }
 
  // Wait for all kernels in the set to finish
  cl_int finish(void) {
    cl_int res, res_out = CL_SUCCESS;
    for (kernel_iter iter = m_kernels.begin(); iter != m_kernels.end(); ++iter) {
      res = clFinish (iter->queue);
      if (res != CL_SUCCESS) {
        printf ("clFinish failed on queue for kernel %s. Error %d\n", iter->kernel_name, res);
        res_out = res;
      }
    }
    return res_out;
  }
  
  // This kernel has no arguments.
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim) {
    cl_int status = setup_kernel (kernel_name, work_dim, global_dim);
    if (status != CL_SUCCESS) {
      return status;
    }
    return status;
  }
  
  // Variadic templates from C++'11 would be nice here...
  template <typename T1>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1) {
    cl_int status = setup_kernel (kernel_name, work_dim, global_dim);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 0, arg1);
    return status;
  }
  
  template <typename T1, typename T2>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 1, arg2);
    return status;
  }
  
  template <typename T1, typename T2, typename T3>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 2, arg3);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 3, arg4);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 4, arg5);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4, arg5);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 5, arg6);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4, arg5, arg6);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 6, arg7);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 7, arg8);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 8, arg9);
    return status;
  }
  
  template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
  cl_int addKernel(const char *kernel_name, cl_uint work_dim, size_t *global_dim, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10) {
    cl_int status = addKernel (kernel_name, work_dim, global_dim, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    if (status != CL_SUCCESS) {
      return status;
    }
    status |= set_kernel_arg (cur_desc.kernel, kernel_name, 9, arg10);
    return status;
  }
  
private:
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
  kernel_desc_vec m_kernels;
  kernel_desc cur_desc; // temporary
  
  cl_int setup_kernel (const char *kernel_name, cl_uint work_dim, size_t *global_dim) {
    cl_int status;
    cl_kernel kernel = clCreateKernel(m_program, kernel_name, &status);
    if (status != CL_SUCCESS) {
      printf("Failed to create kernel %s. Error %d\n", kernel_name, status);
      return status;
    }
    cl_command_queue queue = clCreateCommandQueue(m_context, m_device, CL_QUEUE_PROFILING_ENABLE, &status);
    if(status != CL_SUCCESS) {
      printf("Failed to create queue for kernel %s. Error %d\n", kernel_name, status);
      return status;
    }
    cur_desc.kernel_name = kernel_name;
    cur_desc.kernel = kernel;
    cur_desc.queue = queue;
    cur_desc.work_dim = work_dim;
    cur_desc.global_dim = global_dim;
    
    m_kernels.push_back (cur_desc);
    return status;
  }
  
  template <typename T>
  cl_int set_kernel_arg (cl_kernel kernel, const char *kernel_name, cl_uint arg_number, T arg1) {
    cl_int status = clSetKernelArg(kernel, arg_number, sizeof(T), (void*)&arg1);
    if(status != CL_SUCCESS) {
      printf("Failed to set argument #%d for kernel %s. Error %d\n", arg_number, kernel_name, status);
    }
    return status;
  }
};
