#pragma once

namespace neoslancer {

// The subset of WinMain's command-line switch parsing
// (reversing/reverse_engineered_functions.md lines 96-100) whose switch
// names are directly confirmed in the decompile: "-credits" and
// "-greyscale". A third numeric-suffixed switch is noted there but its
// literal text wasn't recovered, so it's not ported here.
struct CommandLineOptions {
    bool credits = false;
    bool greyscale = false;
};

CommandLineOptions parseCommandLine(int argc, char** argv);

} // namespace neoslancer
