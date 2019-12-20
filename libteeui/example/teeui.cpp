/*
 *
 * Copyright 2019, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "layout.h"
#include <teeui/example/teeui.h>

using namespace teeui;

static uint32_t device_width_px;
static uint32_t device_height_px;
static double dp2px_;
static double mm2px_;

uint32_t alfaCombineChannel(uint32_t shift, double alfa, uint32_t a, uint32_t b) {
    a >>= shift;
    a &= 0xff;
    b >>= shift;
    b &= 0xff;
    double acc = alfa * a + (1 - alfa) * b;
    if (acc <= 0) return 0;
    uint32_t result = acc;
    if (result > 255) return 255 << shift;
    return result << shift;
}

template <typename T> uint32_t renderPixel(uint32_t x, uint32_t y, const T& e) {
    return e.bounds_.drawPoint(Point<pxs>(x, y));
}

template <typename... Elements>
uint32_t renderPixel(uint32_t x, uint32_t y, const std::tuple<Elements...>& layout) {
    uint32_t intensity = (x * 256) / device_width_px;
    uint32_t acc = (intensity & 0xff) << 16 | (intensity & 0xff) << 8 | (intensity & 0xff);
    for (uint32_t value : {renderPixel(x, y, std::get<Elements>(layout))...}) {
        double alfa = (value & 0xff000000) >> 24;
        alfa /= 255.0;
        acc = alfaCombineChannel(0, alfa, value, acc) | alfaCombineChannel(8, alfa, value, acc) |
              alfaCombineChannel(16, alfa, value, acc);
    }
    return acc;
}

struct FrameBuffer {
    uint32_t left_;
    uint32_t top_;
    uint32_t width_;
    uint32_t height_;
    uint32_t* buffer_;
    size_t size_in_elements_;
    uint32_t lineStride_;

    Error drawPixel(uint32_t x, uint32_t y, uint32_t color) const {
        size_t pos = (top_ + y) * lineStride_ + x + left_;
        if (pos >= size_in_elements_) {
            return Error::OutOfBoundsDrawing;
        }
        double alfa = (color & 0xff000000) >> 24;
        alfa /= 255.0;
        auto acc = buffer_[pos];
        buffer_[pos] = alfaCombineChannel(0, alfa, color, acc) |
                       alfaCombineChannel(8, alfa, color, acc) |
                       alfaCombineChannel(16, alfa, color, acc);
        return Error::OK;
    }
};

template <typename... Elements>
Error drawElements(std::tuple<Elements...>& layout, const PixelDrawer& drawPixel) {
    // Error::operator|| is overloaded, so we don't get short circuit evaluation.
    // But we get the first error that occurs. We will still try and draw the remaining
    // elements in the order they appear in the layout tuple.
    return (std::get<Elements>(layout).draw(drawPixel) || ...);
}

uint32_t setDeviceInfo(uint32_t width, uint32_t height, uint32_t colormodel, double dp2px,
                       double mm2px) {
    dp2px_ = dp2px;
    mm2px_ = mm2px;
    (void)colormodel;  // ignored for now;
    device_width_px = width;
    device_height_px = height;
    return 0;
}

uint32_t renderUIIntoBuffer(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t lineStride,
                            uint32_t* buffer, size_t buffer_size_in_elements_not_bytes) {
    uint32_t afterLastPixelIndex = 0;
    if (__builtin_add_overflow(y, h, &afterLastPixelIndex) ||
        __builtin_add_overflow(afterLastPixelIndex, -1, &afterLastPixelIndex) ||
        __builtin_mul_overflow(afterLastPixelIndex, lineStride, &afterLastPixelIndex) ||
        __builtin_add_overflow(afterLastPixelIndex, x, &afterLastPixelIndex) ||
        __builtin_add_overflow(afterLastPixelIndex, w, &afterLastPixelIndex) ||
        afterLastPixelIndex > buffer_size_in_elements_not_bytes) {
        return uint32_t(Error::OutOfBoundsDrawing);
    }
    context<ConUIParameters> conv(mm2px_, dp2px_);
    conv.setParam<RightEdgeOfScreen>(pxs(device_width_px));
    conv.setParam<BottomOfScreen>(pxs(device_height_px));
    conv.setParam<PowerButtonTop>(100_dp);
    conv.setParam<PowerButtonBottom>(150_dp);
    conv.setParam<VolUpButtonTop>(200_dp);
    conv.setParam<VolUpButtonBottom>(250_dp);
    conv.setParam<DefaultFontSize>(14_dp);
    conv.setParam<BodyFontSize>(16_dp);

    auto layoutInstance = instantiateLayout(ConfUILayout(), conv);

    uint32_t* begin = buffer + (y * lineStride + x);
    for (uint32_t yi = 0; yi < h; ++yi) {
        for (uint32_t xi = 0; xi < w; ++xi) {
            begin[xi] = 0xffffffff;
            //            begin[xi] = renderPixel(x + xi, y + yi, layoutInstance);
        }
        begin += lineStride;
    }
    FrameBuffer fb;
    fb.left_ = x;
    fb.top_ = y;
    fb.width_ = w;
    fb.height_ = h;
    fb.buffer_ = buffer;
    fb.size_in_elements_ = buffer_size_in_elements_not_bytes;
    fb.lineStride_ = lineStride;

    auto pixelDrawer = makePixelDrawer(
        [&fb](uint32_t x, uint32_t y, Color color) -> Error { return fb.drawPixel(x, y, color); });

    if (auto error = drawElements(layoutInstance, pixelDrawer)) {
        return uint32_t(error.code());
    }

    return 0;  // OK
}
