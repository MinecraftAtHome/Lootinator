#ifndef LOOTINATOR_GLOBAL_SETTINGS_H
#define LOOTINATOR_GLOBAL_SETTINGS_H

class GlobalSettings {
public:
    static const char* UNSIGNED_32_TYPE = "unsigned int";
    static const char* SIGNED_32_TYPE = "int";
    static const char* UNSIGNED_64_TYPE = "unsigned long long";
    static const char* SIGNED_64_TYPE = "long long";

    static const int THREADS_PER_BLOCK = 256;

    // TODO
    // static void set_from_file();
};

#endif