#include "misc.h"
#include <fstream>
#include <stdexcept>


std::string toLower(std::string str){
    static const char caseDiff = ('A'-'a');
    for(auto it = str.begin(); it != str.end(); ++it){
        if(*it >= 'A' && *it <= 'Z'){
            *it = (*it - caseDiff);
        }
    }
    return str;
}

bool isDigit(char c){
    return (c >= '0') && (c <= '9');
}

void fdump(const std::string &input,
           const std::string &fileName){
    std::ofstream fout(fileName);
    if(!fout){
        throw std::runtime_error("Error opening output file: " + fileName);
    }
    fout << input;
}

size_t findDigit(const std::string& str,
                 size_t pos){
    while(!isDigit(str[pos])){
        pos++;
        if(pos >= str.length()){
            return std::string::npos;
        }
    }
    return pos;
}

size_t extractDigits(const std::string& str,
                     size_t pos){
    size_t result = 0;
    while(isDigit(str[pos])){
        result *= 10;
        result += str[pos++] - '0';
    }
    return result;
}

