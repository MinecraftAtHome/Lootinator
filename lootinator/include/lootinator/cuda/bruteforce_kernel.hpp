#ifndef LOOTINATOR_CUDA_BRUTEFORCE_KERNEL
#define LOOTINATOR_CUDA_BRUTEFORCE_KERNEL

#include "lootinator/cuda/kernel.hpp"

#include <ostream>


namespace cuda {
    class BruteforceKernel : public Kernel {
    public:
        virtual void generate(std::ostream& out) const;
    };
}

#endif