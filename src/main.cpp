// Purpose: Provides the initial entry point and optional data-directory argument.
// Author: Rami Badr
#include "TextUtils.h"
#include <iostream>
#include <string>

// Validates startup arguments; module loading and the required menu follow in later steps.
int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: campus [data-directory]\n";
        return 1;
    }
    const std::string dataDirectory = argc == 2 ? argv[1] : "data/";
    if (TextUtils::trim(dataDirectory).empty()) {
        std::cerr << "The data directory cannot be blank.\n";
        return 1;
    }
    std::cout << "Smart Campus Navigation and Facility Analytics System\n"
              << "Data directory: " << dataDirectory << '\n';
    return 0;
}
