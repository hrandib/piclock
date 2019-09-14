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
#ifndef SENSORS_H
#define SENSORS_H

// It is assumed that all sensors already existed in sysfs-iio

#include "common.h"
#include <chrono>
#include <thread>

enum class SensorType {
    TEMPERATURE,
    HUMIDITY,
    LUMINOSITY,
    PRESSURE,

    MAX_VAL
};

inline const char* const UNITS[size_t(SensorType::MAX_VAL)] = { "℃", "%H", "lux", "hPa" };

struct SensorDescriptor {
    std::string sensorName;
    SensorType sensorType;
    std::string valueName;
    std::filesystem::path sensorPath;
};

const inline std::array<SensorDescriptor, 5> DESCRIPTORS = {{
    { "bh1750", SensorType::LUMINOSITY, "in_illuminance_raw", {} },
    { "bmp280", SensorType::PRESSURE, "x", {} },
    { "bmp280", SensorType::TEMPERATURE, "x", {} },
    { "hdc1080", SensorType::HUMIDITY, "x", {} },
    { "hdc1080", SensorType::TEMPERATURE, "x", {} },
}};


class Sensor {
public:
    using path = std::filesystem::path;
    using string = std::string;
    using string_view = std::string_view;
    using fstream = std::ifstream;
    using Color = YAML::Color;

    Sensor(const SensorDescriptor& desc, PositionType position, Color color) : senseDesc_{desc},
        fstream_{desc.sensorPath/desc.valueName},
        position_{position},
        color_{color}
    {  }

    const string& GetName() {
        return senseDesc_.sensorName;
    }

    SensorType GetType() {
        return senseDesc_.sensorType;
    }

    const path& GetSensorPath() {
        return senseDesc_.sensorPath;
    }

    const string& GetValueName() {
        return senseDesc_.valueName;
    }

    string GetFormattedValue() {
        return ReadValue() + " " + UNITS[size_t(GetType())];
    }

    const PositionType& GetPosition() {
        return position_;
    }

    const Color& GetColor() {
        return color_;
    }

private:
    const SensorDescriptor senseDesc_;
    fstream fstream_;
    const PositionType position_;
    const Color color_;

    string ReadValue() {
        string result;
        try {
            fstream_ >> result;
            fstream_.seekg(0);
        } catch(const std::exception& e) {
            std::cerr << "Read failed for" << GetName() << "  " << e.what() << std::endl;
            fstream_.close();
            fstream_.open(GetSensorPath()/GetValueName());
            if(!fstream_) {
                std::cerr << "Retry open failed" << std::endl;
            }
        }
        return result;
    }
};

using ms = std::chrono::milliseconds;
using namespace std::string_literals;

class SensorHub final : public WidgetWrapper {
private:
    using Node = YAML::Node;
    using fstream = std::ifstream;
    using string = std::string;
    using string_view = std::string_view;
    using dir_iterator = std::filesystem::directory_iterator;
    using Path = std::filesystem::path;
    using PathMap = std::map<string, Path>;
    using Font = rgb_matrix::Font;
    using Color = YAML::Color;

    static constexpr string_view SENSORS_ROOT = "/sys/bus/iio/devices";

    std::vector<Sensor> sensors_;
    Font font_;


public:
    SensorHub(const Options& options, BaseWidget& widget) : WidgetWrapper{widget}, sensors_{} {

        class invalid_argument : public std::invalid_argument {
        public:
            invalid_argument(const string& msg) : std::invalid_argument{"SensorHub -> " + msg}
            { }
        };

        OptionalNode optionalNode = options.GetNode("sensors");
        if(!optionalNode) {
            throw invalid_argument("Configuration node not found");
        }
        Node sensorsNode = *optionalNode;

        const string fontFile = sensorsNode["font"].as<string>();
        Path fontPath = options.GetExecDir() / fontFile;
        if(!font_.LoadFont(fontPath.c_str())) {
            throw invalid_argument("Couldn't load font "s + fontPath.c_str());
        }
        PositionType position;
        try {
            position = sensorsNode["position"].as<PositionType>();
        } catch(const YAML::TypedBadConversion<int32_t>&) {
            throw invalid_argument{"Reading position failed"};
        }

        std::cout << "Font baseline: " << font_.baseline() << std::endl;
        for(const auto& [name, path] : GetAvailableSensors()) {
            std::cout << name << "   " << path.c_str() << std::endl;
            for(const auto& desc : DESCRIPTORS) {
                if (desc.sensorName == name) {
                    auto tempDesc = desc;
                    tempDesc.sensorPath = path;
                    sensors_.emplace_back(tempDesc, position, Color{255, 255, 0});
                    position[1] += font_.height();
                }
            }
        }

        for(auto& sensor : sensors_) {
            std::cout << sensor.GetName() << "   " << (uint32_t)sensor.GetType() << std::endl;
        }

    }

    void Draw(rgb_matrix::FrameCanvas* canvas) final {
        static constexpr size_t letterSpacing = 0;
        for(auto& sensor : sensors_) {
            auto& [xPos, yPos] = sensor.GetPosition();
            Color color = sensor.GetColor();
            string value = sensor.GetFormattedValue();
            rgb_matrix::DrawText(canvas, font_, xPos, yPos + font_.baseline(),
                                 color, nullptr, value.data(),
                                 letterSpacing);
        }
    }

private:
    PathMap GetAvailableSensors() {
        PathMap result;
        for(const Path& dir : dir_iterator{SENSORS_ROOT}) {
            if(string name = GetSensorName(dir); !name.empty()) {
                result[name] = dir;
            }
        }
        return result;
    }

    string GetSensorName(const Path& sensorPath) {
        try {
            fstream file{sensorPath/"name"};
            string result;
            file >> result;
            return result;
        } catch(const std::exception&) {
            return {};
        }
    }

};



#endif // SENSORS_H
