#include <iostream>
#include <string>

#include "streamlens/version.h"

int main() {
    if (std::string(STREAMLENS_VERSION_STRING).empty()) {
        std::cerr << "Version must not be empty\n";
        return 1;
    }

    return 0;
}
