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
#include <cassert>
#include <iostream>
#include <localization/ConfirmationUITranslations.h>
#include <teeui/example/teeui.h>
#include <typeinfo>

using namespace teeui;

static DeviceInfo sDeviceInfo;
static bool sMagnified;

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

uint32_t setDeviceInfo(DeviceInfo deviceInfo, bool magnified) {
    sDeviceInfo = deviceInfo;
    sMagnified = magnified;
    return 0;
}

void selectLanguage(const char* language_id) {
    ConfirmationUITranslations_select_lang_id(language_id);
}

void translate(LabelImpl* label) {
    uint64_t textId = label->textId();
    const char* translation = ConfirmationUITranslations_lookup(textId);
    label->setText({&translation[0], &translation[strlen(translation)]});
}

template <typename... Elements> void translateLabels(std::tuple<Elements...>& layout) {
    translate(&std::get<LabelOK>(layout));
    translate(&std::get<LabelCancel>(layout));
    translate(&std::get<LabelTitle>(layout));
    translate(&std::get<LabelHint>(layout));
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
    context<ConUIParameters> conv(sDeviceInfo.mm2px_, sDeviceInfo.dp2px_);
    conv.setParam<RightEdgeOfScreen>(pxs(sDeviceInfo.width_));
    conv.setParam<BottomOfScreen>(pxs(sDeviceInfo.height_));
    conv.setParam<PowerButtonTop>(mms(sDeviceInfo.powerButtonTopMm_));
    conv.setParam<PowerButtonBottom>(mms(sDeviceInfo.powerButtonBottomMm_));
    conv.setParam<VolUpButtonTop>(mms(sDeviceInfo.volUpButtonTopMm_));
    conv.setParam<VolUpButtonBottom>(mms(sDeviceInfo.volUpButtonBottomMm_));
    if (sMagnified) {
        conv.setParam<DefaultFontSize>(18_dp);
        conv.setParam<BodyFontSize>(20_dp);
    } else {
        conv.setParam<DefaultFontSize>(14_dp);
        conv.setParam<BodyFontSize>(16_dp);
    }

    auto layoutInstance = instantiateLayout(ConfUILayout(), conv);

    translateLabels(layoutInstance);

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
