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

inline const char* const UNITS[size_t(SensorType::MAX_VAL)] = { "C", "%H", "lux", "hPa" };

struct SensorDescriptor {
    std::string sensorName;
    SensorType sensorType;
    std::string valueName;
    std::filesystem::path sensorPath;
};

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
    const PositionType& GetPosition() {
        return position_;
    }
    const Color& GetColor() {
        return color_;
    }
    string GetFormattedValue() {
        return ReadValue() + " " + UNITS[size_t(GetType())];
    }

private:
    const SensorDescriptor senseDesc_;
    fstream fstream_;
    const PositionType position_;
    const Color color_;

    string ReadValue();
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
    SensorHub(const Options& options, BaseWidget& widget);
    void Draw(rgb_matrix::FrameCanvas* canvas) final;

private:
    PathMap GetAvailableSensors();
    string GetSensorName(const Path& sensorPath);

};



#endif // SENSORS_H
