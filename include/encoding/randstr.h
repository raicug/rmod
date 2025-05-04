#ifndef RANDSTR_H
#define RANDSTR_H

#include <random>
#include <string>

class randstr {
public:
    static std::string generate(size_t length);
};

#endif //RANDSTR_H