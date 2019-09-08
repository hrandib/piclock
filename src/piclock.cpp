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

#include "common.h"
#include "clock_impl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

static volatile bool interrupt_received = false;
static void InterruptHandler(int /*signo*/)
{
    interrupt_received = true;
}

using namespace std;

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
    unique_ptr<RGBMatrix> matrix;
    if(auto matrixOpts = opts.GetMatrixOptions(); matrixOpts) {
        auto ptr = rgb_matrix::CreateMatrixFromOptions(*opts.GetMatrixOptions(), *opts.GetRuntimeOptions());
        matrix.reset(ptr);
    }
    if(!matrix) {
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
    write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
    return 0;
}
