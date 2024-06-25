#include "include/one_of.hpp"
#include <cstdint>
#include <iostream>
#include <string>
using namespace ezr;
enum class eText{
    U32, String
};
using Text = one_of<eText, uint32_t, std::string>;
int main()
{
    Text t(eText::U32, "lmao");
    std::cout << t.get<eText::String>() << std::endl;
}