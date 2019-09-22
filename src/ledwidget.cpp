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
#include "ledwidget.h"

using std::unique_lock;

BaseWidget::~BaseWidget() = default;

WidgetWrapper::WidgetWrapper(BaseWidget &widget) : widget_{widget}
{ }

void WidgetWrapper::RequestUpdate() {
    widget_.RequestUpdate();
}

void MainWidget::Draw(rgb_matrix::FrameCanvas *canvas) {
    unique_lock lk{mtx_};
    cv_.wait(lk, [this]{ return pendingRequest_.load();});
    for(WidgetPtr& widget : widgets_) {
        widget->Draw(canvas);
    }
    pendingRequest_ = false;
    lk.unlock();
    cv_.notify_one();
}

void MainWidget::RequestUpdate() {
    unique_lock lk{mtx_};
    cv_.wait(lk, [this] { return !pendingRequest_.load();});
    pendingRequest_ = true;
    lk.unlock();
    cv_.notify_one();
}
