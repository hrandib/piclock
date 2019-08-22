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

using std::string;
using std::vector;
using std::invalid_argument;
using std::to_string;
using std::filesystem::path;
using std::filesystem::exists;
using namespace std::string_literals;
using namespace rgb_matrix;

namespace  {

    string cli_args =
        "app --led-rows=64 --led-cols=64 --led-chain=3 "
        "-f /home/pi/source/rpi-rgb-led-matrix/fonts-aux/hoog24.bdf -b 5 -C 255,255,30 -d %H:%M -x 105";

    bool parseColor(Color* c, const char* str);
    bool FullSaturation(const Color& c);

    const char* time_format = "%H:%M";
    Color color(255, 255, 0);
    Color bg_color(0, 0, 0);
    int x_orig = 0;
    int y_orig = 0;
    int letter_spacing = 0;

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

Clock::Clock(const path& execDir, const YAML::Node& clockNode)
{
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
        auto [xPos, yPos] = clockNode["position"].as<std::array<int, 2>>();
        xPos_ = xPos;
        yPos_ = yPos;
    } catch(const YAML::TypedBadConversion<string>& ) {
        throw invalid_argument{"Error reading font from yaml"};
    } catch(const YAML::TypedBadConversion<int>&) {
        throw invalid_argument{"Error reading position or color from yaml"};
    }
}

void Clock::Update(rgb_matrix::FrameCanvas* canvas)
{
    char text_buffer[32];
    const int x = x_orig;
    int y = y_orig;
    struct timespec next_time;
    next_time.tv_sec = time(nullptr);
    next_time.tv_nsec = 0;
    struct tm tm;

    localtime_r(&next_time.tv_sec, &tm);
    strftime(text_buffer, sizeof(text_buffer), time_format, &tm);
    canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
    rgb_matrix::DrawText(canvas, font_, x, y + font_.baseline(),
                         color, nullptr, text_buffer,
                         letter_spacing);
    // Wait until we're ready to show it.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, nullptr);
    next_time.tv_sec += 1;
}
