/*
 * Copyright (c) 2019 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
using std::filesystem::path;
using std::experimental::source_location;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::RuntimeOptions;

static volatile bool interrupt_received = false;
static void InterruptHandler(int /*signo*/)
{
    interrupt_received = true;
}

using namespace std;

class Options
{
private:
    YAML::Node rootNode_;
    path execDir_;

    optional<YAML::Node> getNode(const string& nodeName) const {
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
        path execPath{programPath};
        execDir_ = execPath.remove_filename();
        auto configPath = execPath.replace_filename(configFileName);
        cout << "Used config file: " << configPath << endl;
        rootNode_ = YAML::LoadFile(configPath);
        if(!rootNode_.IsDefined()) {
            throw YAML::Exception{YAML::Mark::null_mark(), "Root node not found"};
        }
    }

    optional<RGBMatrix::Options> GetMatrixOptions() {
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

    optional<RuntimeOptions> GetRuntimeOptions() {
        return std::make_optional(RuntimeOptions{});
    }

    std::optional<YAML::Node> GetClockNode() const {
        return getNode("clock");
    }

    const path& GetExecDir() const
    {
        return execDir_;
    }
};

int main(int /*argc*/, char* argv[])
{
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    Options opts{argv[0]};
    std::unique_ptr<Clock> clock;
    try {
        clock = std::make_unique<Clock>(opts.GetExecDir(), *opts.GetClockNode());
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << endl;
        return 1;
    }
    RGBMatrix* matrix{};
    if(auto matrixOpts = opts.GetMatrixOptions(); matrixOpts) {
        matrix = rgb_matrix::CreateMatrixFromOptions(*opts.GetMatrixOptions(), RuntimeOptions{});
    }
    if(matrix == nullptr) {
        std::cerr << "The matrix creation failed";
        return 1;
    }
    FrameCanvas* offscreen = matrix->CreateFrameCanvas();

    while(!interrupt_received) {
        offscreen->Fill(0, 0, 0);
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
