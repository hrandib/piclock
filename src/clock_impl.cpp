// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "clock_impl.h"

#include <vector>
#include <string>
#include <stdexcept>
#include <getopt.h>
#include <string.h>
#include <iostream>

using std::string;
using std::vector;
using std::to_string;
using std::filesystem::path;
using std::filesystem::exists;
using namespace std::string_literals;
using namespace rgb_matrix;
using rgb_matrix::Color;

namespace {

    bool parseColor(Color* c, const char* str)
    {
        return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
    }

    bool FullSaturation(const Color& c)
    {
        return (c.r == 0 || c.r == 255)
               && (c.g == 0 || c.g == 255)
               && (c.b == 0 || c.b == 255);
    }

}

namespace YAML {
    template<>
    struct convert<Color> {
      static Node encode(const Color& rhs) {
        Node node;
        node.push_back(rhs.r);
        node.push_back(rhs.g);
        node.push_back(rhs.b);
        return node;
      }

      static bool decode(const Node& node, Color& rhs) {
        if(!node.IsSequence() || node.size() != 3) {
          return false;
        }

        rhs.r = static_cast<uint8_t>(node[0].as<uint32_t>());
        rhs.g = static_cast<uint8_t>(node[1].as<uint32_t>());
        rhs.b = static_cast<uint8_t>(node[2].as<uint32_t>());
        return true;
      }
    };
} //YAML

Clock::Clock(const path& execDir, const YAML::Node& clockNode, BaseWidget& widget) : WidgetWrapper{widget}
{
    class invalid_argument : public std::invalid_argument {
    public:
        invalid_argument(const string& msg) : std::invalid_argument{"Clock -> " + msg}
        { }
    };

    try {
        const string fontFile = clockNode["font"].as<string>();
        path fontPath = execDir / fontFile;
        std::error_code ec;
        if(!exists(fontPath, ec)) {
            throw invalid_argument{"Font file doesn't exist: "s + fontPath.c_str()};
        }
        if(!font_.LoadFont(fontPath.c_str())) {
            throw invalid_argument("Couldn't load font "s + fontPath.c_str());
        }
        position_ = clockNode["position"].as<PositionType>();
        color_ = clockNode["color"].as<Color>();
        timeFormat_ = clockNode["format"].as<string>();

    } catch(const YAML::TypedBadConversion<string>& ) {
        throw invalid_argument{"Error reading font from yaml"};
    } catch(const YAML::TypedBadConversion<int32_t>&) {
        throw invalid_argument{"Error reading position from yaml"};
    } catch(const YAML::TypedBadConversion<uint32_t>&) {
        throw invalid_argument{"Error reading color from yaml"};
    }
}

void Clock::Draw(rgb_matrix::FrameCanvas* canvas)
{
    static const int letterSpacing = 0;
    char text_buffer[32];

    struct timespec next_time;
    next_time.tv_sec = time(nullptr);
    next_time.tv_nsec = 0;
    struct tm tm;

    localtime_r(&next_time.tv_sec, &tm);
    strftime(text_buffer, sizeof(text_buffer), timeFormat_.data(), &tm);

    rgb_matrix::DrawText(canvas, font_, position_[0], position_[1] + font_.baseline(),
                         color_, nullptr, text_buffer,
                         letterSpacing);
    // Wait until we're ready to show it.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, nullptr);
    next_time.tv_sec += 1;
}
