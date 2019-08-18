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
using namespace std::string_literals;
using namespace rgb_matrix;

int brightness = 10;

namespace  {

    string cli_args =
        "app --led-rows=64 --led-cols=64 --led-chain=3 "
        "-f /home/pi/source/rpi-rgb-led-matrix/fonts-aux/hoog32.bdf -b 5 -C 255,255,30 -d %H:%M -x 50";

    int usage(const char* progname);
    bool parseColor(Color* c, const char* str);
    bool FullSaturation(const Color& c);
    vector<const char*> simpleSplit(string& str);

    const char* time_format = "%H:%M";
    Color color(255, 255, 0);
    Color bg_color(0, 0, 0);
    int x_orig = 0;
    int y_orig = 0;
    int letter_spacing = 0;

    int usage(const char* progname)
    {
        fprintf(stderr, "usage: %s [options]\n", progname);
        fprintf(stderr, "Reads text from stdin and displays it. "
                "Empty string: clear screen\n");
        fprintf(stderr, "Options:\n");
        rgb_matrix::PrintMatrixFlags(stderr);
        fprintf(stderr,
                "\t-d <time-format>  : Default '%%H:%%M'. See strftime()\n"
                "\t-f <font-file>    : Use given font.\n"
                "\t-b <brightness>   : Sets brightness percent. Default: 100.\n"
                "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
                "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
                "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
                "\t-C <r,g,b>        : Color. Default 255,255,0\n"
               );
        return 1;
    }

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

    vector<const char*> simpleSplit(string& str)
    {
        vector<const char*> tokens;
        if(!str.empty()) {
            tokens.push_back(str.data());
            for(size_t pos{}; pos != string::npos;) {
                pos = str.find(" ", pos);
                if(pos != string::npos) {
                    str[pos++] = '\0';
                    tokens.push_back(&str[pos]);
                }
            }
        }
        return tokens;
    }
}

rgb_matrix::RGBMatrix::Options Clock::matrixOptions() const
{
    return matrixOptions_;
}

rgb_matrix::RuntimeOptions Clock::runtimeOptions() const
{
    return runtimeOptions_;
}

Clock::Clock(char** argv)
{
    auto illegalOption = [&argv](auto msg) {
        usage(argv[0]);
      throw invalid_argument{msg};
    };
    auto tokens_vec = simpleSplit(cli_args);
    int tokens_size = static_cast<int>(tokens_vec.size());
    char** tokens = const_cast<char**>(tokens_vec.data());
    if(!rgb_matrix::ParseOptionsFromFlags(&tokens_size, &tokens,
                                          &matrixOptions_, &runtimeOptions_)) {
        illegalOption("Args parsing failed");
    }

    const char* bdf_font_file = nullptr;

    int opt;
    while((opt = getopt(tokens_size, tokens, "x:y:f:C:B:O:b:S:d:")) != -1) {
        switch(opt) {
        case 'd':
            time_format = strdup(optarg);
            break;
        case 'b':
            brightness = atoi(optarg);
            break;
        case 'x':
            x_orig = atoi(optarg);
            break;
        case 'y':
            y_orig = atoi(optarg);
            break;
        case 'f':
            bdf_font_file = strdup(optarg);
            break;
        case 'S':
            letter_spacing = atoi(optarg);
            break;
        case 'C':
            if(!parseColor(&color, optarg)) {
                illegalOption("Invalid color spec");
            }
            break;
        default:
            illegalOption("No such option: " + to_string(opt));
        }
    }
    if(bdf_font_file == nullptr) {
        illegalOption("Need to specify BDF font-file with -f");
    }
    /*
     * Load font. This needs to be a filename with a bdf bitmap font.
     */
    if(!font_.LoadFont(bdf_font_file)) {
        illegalOption("Couldn't load font "s + bdf_font_file);
    }
    if(brightness < 1 || brightness > 100) {
        illegalOption("Brightness is outside usable range");
    }
}

void Clock::Update(rgb_matrix::FrameCanvas* canvas)
{
    const int x = x_orig;
    int y = y_orig;
    struct timespec next_time;
    next_time.tv_sec = time(nullptr);
    next_time.tv_nsec = 0;
    struct tm tm;

    char text_buffer[256];
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
