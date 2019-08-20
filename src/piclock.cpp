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
#include <optional>
#include <experimental/source_location>

using std::string;
using std::pair;
using std::vector;
using std::cout;
using std::endl;
using std::optional;
using std::experimental::source_location;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::RuntimeOptions;

static volatile bool interrupt_received = false;
static void InterruptHandler(int /*signo*/)
{
    interrupt_received = true;
}

class Options
{
private:
    YAML::Node rootNode_;

    optional<YAML::Node> getNode(const string& nodeName) {
        try {
            YAML::Node node = rootNode_[nodeName];
            if(node.IsDefined()) {
                return std::make_optional(node);
            }
        } catch(const YAML::InvalidNode&) {
            return {};
        }
        return {};
    }

public:
    Options(const char* programPath, const char* configFileName = "config.yml") {
        std::filesystem::path fspath{programPath};
        fspath.replace_filename(configFileName);
        cout << "Used config file: " << fspath << '\n';
        rootNode_ = YAML::LoadFile(fspath);
        if(!rootNode_.IsDefined()) {
            throw YAML::Exception{YAML::Mark::null_mark(), "Root node not found"};
        }
    }
    optional<RGBMatrix::Options> getMatrixOptions() {
        if(auto optionalNode = getNode("matrix"); optionalNode) {
            try {
                auto node = optionalNode.value();
                int rows = node["rows"].as<int>();
                int cols = node["cols"].as<int>();
                int chain = node["chain"].as<int>();
                int brigthness = node["brightness"].as<int>();
                RGBMatrix::Options options;
                options.rows = rows;
                options.cols = cols;
                options.chain_length = chain;
                options.brightness = brigthness;
                if(options.Validate(nullptr)) {
                    return std::make_optional(options);
                }
            } catch (const YAML::Exception&) {
                return {};
            }
        }
        return {};
    }
};


int main(int /*argc*/, char* argv[])
{
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    Options opts{argv[0]};
    std::unique_ptr<Clock> clock;
    try {
        clock = std::make_unique<Clock>(argv);
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        return 1;
    }
    RGBMatrix* matrix{};
    if(auto matrixOpts = opts.getMatrixOptions(); matrixOpts) {
        matrix = rgb_matrix::CreateMatrixFromOptions(*opts.getMatrixOptions(), RuntimeOptions{});
    }
    if(matrix == nullptr) {
        std::cerr << "The matrix creation failed";
        return 1;
    }
    FrameCanvas* offscreen = matrix->CreateFrameCanvas();

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
