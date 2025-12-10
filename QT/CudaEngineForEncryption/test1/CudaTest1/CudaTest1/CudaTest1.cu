#include <iostream>
#include <cuda_runtime.h>
#include <chrono>

__global__ void add(int* a, int* b, int* c, int n, int iterations) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n) {
        int result = 0;
        for (int iter = 0; iter < iterations; iter++) {
            result = a[index] + b[index];
        }
        c[index] = result;
    }
}

void addCPU(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
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
    const int iterations = 10000000;
    
    // Allocate host memory
    int* h_a = new int[N];
    int* h_b = new int[N];
    int* h_c_gpu = new int[N];
    int* h_c_cpu = new int[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        h_a[i] = i;
        h_b[i] = i * 2;
    }

    // ==================== CPU BENCHMARK ====================
    std::cout << "\n========== CPU Benchmark ==========" << std::endl;
    auto cpu_start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; iter++) {
        addCPU(h_a, h_b, h_c_cpu, N);
    }
    
    auto cpu_end = std::chrono::high_resolution_clock::now();
    auto cpu_duration = std::chrono::duration_cast<std::chrono::milliseconds>(cpu_end - cpu_start);
    std::cout << "CPU Time: " << cpu_duration.count() << " ms" << std::endl;

    // ==================== GPU BENCHMARK ====================
    std::cout << "\n========== GPU Benchmark ==========" << std::endl;
    
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

    auto gpu_start = std::chrono::high_resolution_clock::now();
    
    // Run kernel once with iterations inside
    add<<<blocksPerGrid, threadsPerBlock>>>(d_a, d_b, d_c, N, iterations);
    
    checkCudaError(cudaGetLastError(), "kernel launch");

    // Wait for GPU to finish
    checkCudaError(cudaDeviceSynchronize(), "device sync");
    
    auto gpu_end = std::chrono::high_resolution_clock::now();
    auto gpu_duration = std::chrono::duration_cast<std::chrono::milliseconds>(gpu_end - gpu_start);
    std::cout << "GPU Time: " << gpu_duration.count() << " ms" << std::endl;

    // Copy result back to CPU
    checkCudaError(cudaMemcpy(h_c_gpu, d_c, bytes, cudaMemcpyDeviceToHost), "copy result to host");

    // ==================== RESULTS ====================
    std::cout << "\n========== Performance Comparison ==========" << std::endl;
    std::cout << "CPU Time: " << cpu_duration.count() << " ms" << std::endl;
    std::cout << "GPU Time: " << gpu_duration.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (float)cpu_duration.count() / gpu_duration.count() << "x" << std::endl;
    
    // Verify results match
    bool resultsMatch = true;
    for (int i = 0; i < N; i++) {
        if (h_c_cpu[i] != h_c_gpu[i]) {
            resultsMatch = false;
            break;
        }
    }
    std::cout << "Results match: " << (resultsMatch ? "YES" : "NO") << std::endl;

    // Print first 10 results
    std::cout << "\nFirst 10 results:" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << h_a[i] << " + " << h_b[i] << " = " << h_c_gpu[i] << std::endl;
    }
    
    std::cout << "\nLast 10 results:" << std::endl;
    for (int i = N - 10; i < N; i++) {
        std::cout << h_a[i] << " + " << h_b[i] << " = " << h_c_gpu[i] << std::endl;
    }

    // Free GPU memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    
    // Free host memory
    delete[] h_a;
    delete[] h_b;
    delete[] h_c_gpu;
    delete[] h_c_cpu;

    return 0;
}