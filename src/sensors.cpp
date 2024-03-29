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

#include "sensors.h"

using std::literals::operator""s;

static const std::array<SensorDescriptor, 5> DESCRIPTORS = {{
    { "bh1750", "bh1750", SensorType::LUMINOSITY, "in_illuminance_raw", {} },
    { "bmp280", "bmp280", SensorType::PRESSURE, "in_pressure_input", {} },
    { "bmp280", "bmp280", SensorType::TEMPERATURE, "in_temp_input", {} },
    { "hdc1080", "1-0040", SensorType::HUMIDITY, "in_humidityrelative_raw", {} },
    { "hdc1080", "1-0040", SensorType::TEMPERATURE, "in_temp_raw", {} },
}};

void Sensor::ReadValue() {
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
    value_ = result;
}

SensorHub::SensorHub(const Options& options, BaseWidget& widget) : WidgetWrapper{widget}, sensors_{} {
    const Node sensorsNode = GetSensorsNode(options);
    PositionType startPosition = GetPosition(sensorsNode);
    InitSensors(sensorsNode, startPosition);
    pollThd_ = std::thread{&SensorHub::PollThread, this};
    pollThd_.detach();
}

void SensorHub::PollThread() {
    while(true) {
        for(auto& sensor : sensors_) {
            sensor.ReadValue();
        }
        RequestUpdate();
        std::this_thread::sleep_for(10s);
    }
}

SensorHub::Node SensorHub::GetSensorsNode(const Options& options)
{
    OptionalNode optionalNode = options.GetNode("sensors");
    if(!optionalNode) {
        throw invalid_argument("Configuration node not found");
    }
    return *optionalNode;
}

void SensorHub::InitFont(const Options& options, const Node& sensorsNode)
{
    const string fontFile = sensorsNode["font"].as<string>();
    Path fontPath = options.GetExecDir() / fontFile;
    if(!font_.LoadFont(fontPath.c_str())) {
        throw invalid_argument("Couldn't load font "s + fontPath.c_str());
    }
}

PositionType SensorHub::GetPosition(const Node& sensorsNode)
{
    try {
        return sensorsNode["position"].as<PositionType>();
    } catch(const YAML::TypedBadConversion<int32_t>&) {
        throw invalid_argument{"Reading position failed"};
    }
}

void SensorHub::InitSensors(const Node& sensorsNode, PositionType position)
{
    try{
        for(const auto& [name, path] : GetAvailableSensors()) {
            std::cout << name << "   " << path.c_str() << std::endl;
            for(const auto& desc : DESCRIPTORS) {
                if (desc.sensorName == name) {
                    Color color = sensorsNode[name].as<Color>();
                    auto tempDesc = desc;
                    tempDesc.sensorPath = path;
                    sensors_.emplace_back(tempDesc, position, color);
                    position[1] += font_.height();
                }
            }
        }
    } catch(const YAML::TypedBadConversion<uint32_t>&) {
        throw invalid_argument{"Error reading color from yaml"};
    }
}

void SensorHub::Draw(rgb_matrix::FrameCanvas* canvas) {
    static constexpr size_t letterSpacing = 0;
    for(auto& sensor : sensors_) {
        auto& [xPos, yPos] = sensor.GetPosition();
        Color color = sensor.GetColor();
        string value = sensor.GetFormattedValue();
        rgb_matrix::DrawText(canvas, font_, xPos, yPos + font_.baseline(),
                color, nullptr, value.data(), letterSpacing);
    }
}

SensorHub::PathMap SensorHub::GetAvailableSensors() {
    PathMap result;
    for(const Path& dir : dir_iterator{SENSORS_ROOT}) {
        if(string name = GetSensorName(dir); !name.empty()) {
            result[name] = dir;
        }
    }
    return result;
}

SensorHub::string SensorHub::GetSensorName(const Path& sensorPath) {
    try {
        fstream file{sensorPath/"name"};
        string result;
        file >> result;
        return result;
    } catch(const std::exception&) {
        return {};
    }
}

SensorHub::invalid_argument::invalid_argument(const string& msg) : std::invalid_argument{"SensorHub -> " + msg}
{ }
