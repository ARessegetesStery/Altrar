#pragma once

// Defines the logger help functions in ATR
// All Logging functions should be inlined as this header is included multiple times throughout the project

#include "ATRType.h"
#include "ATROSSpec.h"

#include <iostream>
#include <iomanip>

namespace ATR
{

#if not defined ATR_OUTPUT_SILENT

#define ATR_PRINT(x) {std::cout << x << std::endl;}

#define ATR_LOG(x) {std::cout << ">> " << x << std::endl;}
#define ATR_LOG_PART(name) {std::cout << "======================" << \
	std::left << std::setfill('=') << std::setw(60) << name << std::endl;}
#define ATR_LOG_SECTION(name) {std::cout << "----------------------" << \
	std::left << std::setfill('-') << std::setw(60) << name << std::endl;}

#define ATR_ERROR(x) { \
	OS_ChangeConsoleColor(12); \
	std::cout << "[ERROR] "; \
	OS_ChangeConsoleColor(7); \
	std::cout << x << std::endl;}

// The Logged `x` must be of type which has operator<< defined. Do not append the lagging '\n' to the output of `x`
#define ATR_PEEK(x) {std::cout << "| Variable: " << #x << " " << "| Type: " << typeid(x).name() << std::endl;\
	std::cout << "| Value:" << std::endl << x << std::endl; }

#endif

	struct Format
	{
		static inline String indent = "  - ";
		static inline String smallIndent = "- ";
	};

}
