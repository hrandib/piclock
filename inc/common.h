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
#ifndef COMMON_H
#define COMMON_H

#include "led-matrix.h"
#include "graphics.h"
#include "yaml-cpp/yaml.h"
#include "options.h"
#include "ledwidget.h"

using PositionType = std::array<int32_t, 2>;

namespace YAML {

    using rgb_matrix::Color;

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

#endif // COMMON_H
