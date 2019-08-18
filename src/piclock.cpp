#include "led-matrix.h"
#include "graphics.h"
#include "yaml-cpp/yaml.h"
#include "clock_impl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#undef __cplusplus
#define __cplusplus 201703L
#include <filesystem>
#include <stdexcept>
#include <memory>

using std::string;
using std::pair;
using std::vector;
using std::cout;
using std::endl;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

static volatile bool interrupt_received = false;
static void InterruptHandler(int /*signo*/)
{
    interrupt_received = true;
}


static void loadConfig(const char* execPath)
{
    std::filesystem::path fspath{execPath};
    fspath.replace_filename("config.yml");
    cout << fspath << std::endl;
    YAML::Node root = YAML::LoadFile(fspath);
    if(root.IsDefined()) {
        YAML::Node clock = root["clock"];
        try {
            cout << clock["font"].as<string>() << endl;
        } catch (std::exception& e) {
            cout << e.what() << endl;
        }
    }
}

int main(int /*argc*/, char* argv[])
{
    extern int brightness;

    loadConfig(argv[0]);
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    std::unique_ptr<Clock> clock;
    try {
        clock = std::make_unique<Clock>(argv);
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        return 1;
    }

    RGBMatrix* matrix = rgb_matrix::CreateMatrixFromOptions(clock->matrixOptions(),
                        clock->runtimeOptions());
    if(matrix == nullptr) {
        std::cerr << "The matrix creation failed";
        return 1;
    }
    FrameCanvas* offscreen = matrix->CreateFrameCanvas();
    matrix->SetBrightness(static_cast<uint8_t>(5));

    while(!interrupt_received) {
        clock->Update(offscreen);
        // Atomic swap with double buffer
        offscreen = matrix->SwapOnVSync(offscreen);
    }
    // Finished. Shut down the RGB matrix.
    matrix->Clear();
    delete matrix;
    write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
    return 0;
}
