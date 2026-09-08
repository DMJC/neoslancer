#include "neoslancer/CommandLine.h"

#include <cstring>

namespace neoslancer {

CommandLineOptions parseCommandLine(int argc, char** argv) {
    CommandLineOptions options;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-credits") == 0) {
            options.credits = true;
        } else if (std::strcmp(argv[i], "-greyscale") == 0) {
            options.greyscale = true;
        }
    }
    return options;
}

} // namespace neoslancer
