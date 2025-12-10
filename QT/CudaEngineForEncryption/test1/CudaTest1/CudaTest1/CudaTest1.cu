#include <iostream>
#include <cuda_runtime.h>

__global__ void add(int* a, int* b, int* c, int n) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n) {
        c[index] = a[index] + b[index];
    }
}

void checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        std::cerr << "CUDA Error: " << msg << " - " << cudaGetErrorString(err) << std::endl;
        exit(1);
    }
}

int main() {
    // Check GPU capabilities
    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);
    std::cout << "Found " << deviceCount << " CUDA device(s)." << std::endl;
    
    if (deviceCount > 0) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, 0);
        std::cout << "Device: " << prop.name << std::endl;
        std::cout << "Compute Capability: " << prop.major << "." << prop.minor << std::endl;
    }
    
    int a = 5, b = 7, c = 0;

    int* d_a, * d_b, * d_c;

    // allocate memory on GPU
    checkCudaError(cudaMalloc((void**)&d_a, sizeof(int)), "malloc d_a");
    checkCudaError(cudaMalloc((void**)&d_b, sizeof(int)), "malloc d_b");
    checkCudaError(cudaMalloc((void**)&d_c, sizeof(int)), "malloc d_c");

    // copy from CPU to GPU
    checkCudaError(cudaMemcpy(d_a, &a, sizeof(int), cudaMemcpyHostToDevice), "copy a to device");
    checkCudaError(cudaMemcpy(d_b, &b, sizeof(int), cudaMemcpyHostToDevice), "copy b to device");

    // run kernel with 1 block and 1 thread
    add<<<1, 1>>>(d_a, d_b, d_c);
    checkCudaError(cudaGetLastError(), "kernel launch");

    // wait for GPU to finish
    checkCudaError(cudaDeviceSynchronize(), "device sync");

    // copy result back to CPU
    checkCudaError(cudaMemcpy(&c, d_c, sizeof(int), cudaMemcpyDeviceToHost), "copy result to host");

    // print result
    std::cout << "Result: " << c << std::endl;

    // free GPU memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    return 0;
}
