//#define EXR_ASSERT(X, S) for a custom assertion macro, X is condition and S is message
#pragma once
#ifndef EZR_ASSERT
#include <cassert>
#define EZR_ASSERT(X, S) assert(X && S)
#endif