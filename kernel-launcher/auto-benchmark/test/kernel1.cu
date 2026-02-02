#ifndef SHARED_DEFINITIONS
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

constexpr u64 JRAND_MULTIPLIER = 0x5deece66d;
constexpr u64 MASK_48 = ((1ULL << 48) - 1);

__device__ inline void setSeed(uint64_t* rand, uint64_t value){ *rand = (value ^ 0x5deece66d) & ((1ULL << 48) - 1); }
__device__ inline int next(uint64_t* rand, const int bits){ *rand = (*rand * 0x5deece66d + 0xb) & ((1ULL << 48) - 1); return (int)((int64_t)*rand >> (48 - bits)); }
__device__ inline int nextInt(uint64_t* rand, const int n){ if ((n-1 & n) == 0) {uint64_t x = n * (uint64_t)next(rand, 31); return (int)((int64_t)x >> 31);} else {return (int)(next(rand, 31) % n);} }
__device__ inline float nextFloat(uint64_t* rand){ return next(rand, 24) / (float)(1 << 24) }; 
__device__ inline int nextIntBounded(uint64_t* rand, const int min, const int max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline int nextIntNoAdvance(uint64_t *rand, const int n) {if ((n-1 & n) == 0) {uint64_t x = n * *rand; return (int)((int64_t)x >> 31);} else {return (int)(*rand % n);}} 
#endif

extern "C" {
// end of base class

    // start of StatePredictionSingleItemKernelWhatever
    __global__ void state_prediction_rolls( 
        u64* result_array, u32* result_count, 
        u32* shared_mem_contents, u32 shared_mem_contents_length, 
        u64 offset)
    {
        extern __shared__ u32 data[];
        if (threadIdx.x < shared_mem_contents_length) {
            for (int i = threadIdx.x; i < shared_mem_contents_length; i += blockDim.x) {
                data[i] = shared_mem_contents[i];
            }
        }
        __syncthreads();
        
        const u64 tid = blockIdx.x * blockDim.x + threadIdx.x + offset;
        u64 base_state = tid * (5U << 17);

        for (u32 rem = 2U; rem < 5U; rem++) {
            u64 state = base_state + rem<<17;
            u64* rand = &state;

            int counter = 0;
            for (u32 r = 0; r < rem+4; r++) {
                int item = data[nextInt(rand, 28)];
                if (item == 3)
                    counter += nextInt(rand, 2) + 1;
                else if (item < 2) {
                    state = (state * JRAND_MULTIPLIER + 11) & MASK_48;
                }
            }

            if (counter >= 11) {
                u32 ix = atomicAdd(result_count, 1);
                result_array[ix] = tid ^ JRAND_MULTIPLIER;
            }
        }
    }

} // remember about this guy!