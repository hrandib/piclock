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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "yaml-cpp/yaml.h"
#include "led-matrix.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <memory>
#include <optional>
#include <string>
#include <iostream>

class Options
{
private:
    template<typename T>
    using optional = std::optional<T>;
    using path = std::filesystem::path;
    using string = std::string;
    using RGBMatrix = rgb_matrix::RGBMatrix;
    using RuntimeOptions = rgb_matrix::RuntimeOptions;

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
        std::cout << "Used config file: " << configPath << std::endl;
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


#endif // OPTIONS_H
