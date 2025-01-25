#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

inline int UTF8_CHAR_LEN(char byte) {return byte == 0x0d ? 2 : (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1; }


#endif // HELPER
