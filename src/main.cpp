#include <iostream>

#include "streamlens/version.h"

int main() {
    std::cout << "StreamLens " << STREAMLENS_VERSION_STRING << '\n';
    std::cout << "Video stream relay and diagnostics toolkit\n";
    return 0;
}
