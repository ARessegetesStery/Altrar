#pragma once

// Defines the logger help functions in ATR
// All Logging functions should be inlined as this header is included multiple times throughout the project

#include "ATRType.h"

#include <iostream>

namespace ATR
{

#if defined ATR_DEBUG

#define ATR_COLOR_NC "\e[0m"
#define ATR_COLOR_RED "\e[0;31m"
#define ATR_COLOR_GRN "\e[0;32m"
#define ATR_COLOR_CYN "\e[0;36m"
#define ATR_COLOR_REDB "\e[41m"

#define ATR_PRINT(x) {std::cout << x << std::endl;}

// The Logged `x` must be of type which has operator<< defined. Do not append the lagging '\n' to the output of `x`
#define ATR_LOG(x) {std::cout << "| Variable: " << #x << " " << "| Type: " << typeid(x).name() << std::endl;\
	std::cout << "| Value:" << std::endl << x << std::endl; }

#define ATR_LOG_PART(name) {std::cout << "======================" << name << "======================" << std::endl;}
#define ATR_LOG_SECTION(name) {std::cout << "----------------------" << name << "----------------------" << std::endl;}

#define ATR_ERROR(x) {std::cerr << ATR_COLOR_REDB << "[ERROR] " << ATR_COLOR_NC << x << std::endl;}

#elif defined ATR_RELEASE
#define ATR_LOG(x) {}
#define ATR_LOG_SECTION(name) {}

#endif



}
