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
#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include "led-matrix.h"
#include <mutex>
#include <memory>

struct BaseWidget {
    virtual void Draw(rgb_matrix::FrameCanvas* canvas) = 0;
    virtual void RequestUpdate() = 0;
    virtual ~BaseWidget();
};

using WidgetPtr = std::unique_ptr<BaseWidget>;

class WidgetWrapper : public BaseWidget {
public:
    WidgetWrapper(BaseWidget& widget);
    void RequestUpdate() final;
private:
    BaseWidget& widget_;
};

class MainWidget : public BaseWidget {
public:
    using WidgetVector = std::vector<WidgetPtr>;

    void AddWidget(WidgetPtr& widget) {
        widgets_.push_back(std::move(widget));
    }

    template<typename... Widgets>
    void AddWidgets(WidgetPtr& widget, Widgets&&... widgets) {
        AddWidget(widget);
        if constexpr(sizeof...(widgets) > 0) {
            AddWidgets(std::forward<Widgets>(widgets)...);
        }
    }

    void Draw(rgb_matrix::FrameCanvas *canvas) final {
        for(WidgetPtr& widget : widgets_) {
            widget->Draw(canvas);
        }
    }

    void RequestUpdate() final {

    }

private:
    WidgetVector widgets_{};
    std::mutex mtx_{};
};

#endif // LEDWIDGET_H
