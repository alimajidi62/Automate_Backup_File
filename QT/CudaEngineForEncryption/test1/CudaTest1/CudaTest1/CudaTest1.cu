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
    
    // Define array size
    const int N = 1024;
    const int bytes = N * sizeof(int);
    
    // Allocate host memory
    int* h_a = new int[N];
    int* h_b = new int[N];
    int* h_c = new int[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        h_a[i] = i;
        h_b[i] = i * 2;
    }

    int* d_a, * d_b, * d_c;

    // Allocate memory on GPU
    checkCudaError(cudaMalloc((void**)&d_a, bytes), "malloc d_a");
    checkCudaError(cudaMalloc((void**)&d_b, bytes), "malloc d_b");
    checkCudaError(cudaMalloc((void**)&d_c, bytes), "malloc d_c");

    // Copy from CPU to GPU
    checkCudaError(cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice), "copy a to device");
    checkCudaError(cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice), "copy b to device");

    // Configure kernel launch parameters
    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;
    
    std::cout << "Launching kernel with " << blocksPerGrid << " blocks and " 
              << threadsPerBlock << " threads per block" << std::endl;

    // Run kernel with multiple blocks and threads
    for (size_t i = 0; i < 10000000; i++)
    {
        add << <blocksPerGrid, threadsPerBlock >> > (d_a, d_b, d_c, N);
    }
    
    checkCudaError(cudaGetLastError(), "kernel launch");

    // Wait for GPU to finish
    checkCudaError(cudaDeviceSynchronize(), "device sync");

    // Copy result back to CPU
    checkCudaError(cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost), "copy result to host");

    // Print first 10 and last 10 results
    std::cout << "\nFirst 10 results:" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << h_a[i] << " + " << h_b[i] << " = " << h_c[i] << std::endl;
    }
    
    std::cout << "\nLast 10 results:" << std::endl;
    for (int i = N - 10; i < N; i++) {
        std::cout << h_a[i] << " + " << h_b[i] << " = " << h_c[i] << std::endl;
    }

    // Free GPU memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    
    // Free host memory
    delete[] h_a;
    delete[] h_b;
    delete[] h_c;

    return 0;
}