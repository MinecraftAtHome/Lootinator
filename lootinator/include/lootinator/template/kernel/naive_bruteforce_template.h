#ifndef LOOTINATOR_TEMPLATE_KERNEL_NAIVE_BRUTEFORCE_TEMPLATE_H
#define LOOTINATOR_TEMPLATE_KERNEL_NAIVE_BRUTEFORCE_TEMPLATE_H

#include "lootinator/template/kernel_template.h"


namespace loot {
    class NaiveBruteforceTemplate : KernelTemplate {
    protected:
        virtual void generate_kernel_body(std::ostream& out) const = 0;
    };
}

#endif