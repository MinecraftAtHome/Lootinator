#ifndef LOOTINATOR_CUDA_KERNEL
#define LOOTINATOR_CUDA_KERNEL

namespace cuda {
    class Kernel {
    private:
        virtual void generate_headers();

        // ...
        //virtual void generate(std::ostream& out) const;
    };
}

#endif