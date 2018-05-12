#ifndef MISC_H
#define MISC_H
#include <iostream>
#include <cstdint>

static constexpr uint32_t hash32_helper(const char * const str, uint32_t hash){
    // constexpr uint32_t FNV_PRIME = 16777619;
    return *str ? hash32_helper(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619) : hash;
}

// FNV-1a 32 bit hash
constexpr uint32_t hash32(const char * const str){
    // constexpr uint32_t OFFSET_BASIS = 2166136261;
    return hash32_helper(str, 2166136261);
}


std::string toLower(std::string str);
bool isDigit(char c);

void fdump(std::istream& input,
           const std::string& fileName);

void fdump(const std::string& input,
           const std::string& fileName);

size_t findDigit(const std::string& str,
                 size_t pos);

// handles only digits, so unsigned ints
size_t extractDigits(const std::string& str,
                     size_t pos);


#endif // MISC_H
