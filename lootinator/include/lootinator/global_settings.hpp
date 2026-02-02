#ifndef LOOTINATOR_GLOBAL_SETTINGS_H
#define LOOTINATOR_GLOBAL_SETTINGS_H

class Settings {
public:
    const char* UNSIGNED_32_TYPE = "unsigned int";
    const char* SIGNED_32_TYPE = "int";
    const char* UNSIGNED_64_TYPE = "unsigned long long";
    const char* SIGNED_64_TYPE = "long long";

    const unsigned int THREADS_PER_BLOCK = 256;

    // TODO
    // static void set_from_file();
};

extern Settings global_settings;

#endif