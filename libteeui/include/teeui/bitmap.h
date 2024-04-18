/*
 *
 * Copyright 2022, The Android Open Source Project
 * Copyright 2022 NXP
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

#ifndef LIBTEEUI_BITMAP_H_
#define LIBTEEUI_BITMAP_H_

#include "utils.h"

#include <teeui/error.h>

namespace teeui {

using std::ifstream;
using std::string;

typedef uint32_t       DWORD;
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef uint16_t       WORD;

class BitmapBuffer {
    const uint8_t* data_;
    size_t size_;
public:
    constexpr BitmapBuffer() : data_(nullptr), size_(0) {}
    constexpr BitmapBuffer(const uint8_t* data, size_t size) noexcept : data_(data), size_(size) {}
    template <size_t size>
    explicit constexpr BitmapBuffer(const uint8_t (&data)[size]) noexcept
        : data_(&data[0]), size_(size) {}
    constexpr BitmapBuffer(const BitmapBuffer&) noexcept = default;
    constexpr BitmapBuffer(BitmapBuffer&&) noexcept = default;
    BitmapBuffer& operator=(BitmapBuffer&&) noexcept = default;
    BitmapBuffer& operator=(const BitmapBuffer&) noexcept = default;

    constexpr operator bool() const { return data_ != nullptr; }

    const uint8_t* data() const { return data_; }
    size_t size() const { return size_; }
};

class ScaleBitmapBuffer {
public:
    uint8_t* scale_data_ = nullptr;
    size_t scale_size_ = 0;

    ScaleBitmapBuffer() : scale_data_(nullptr) ,scale_size_(0) {}
    constexpr ScaleBitmapBuffer(uint8_t* scale_data, size_t scale_size) noexcept
            : scale_data_(scale_data), scale_size_(scale_size) {}

    ScaleBitmapBuffer(const ScaleBitmapBuffer&) noexcept = default;
    ScaleBitmapBuffer(ScaleBitmapBuffer&& other) noexcept {
        if (other.scale_data_ != nullptr) {
            scale_data_ = other.scale_data_;
            other.scale_data_ = nullptr;
            scale_size_ = other.scale_size_;
        }
    }
    ScaleBitmapBuffer& operator=(const ScaleBitmapBuffer&) noexcept = default;
    ScaleBitmapBuffer& operator=(ScaleBitmapBuffer&& other) noexcept {
        if (this != &other) {
            if (scale_data_ != nullptr) {
                delete []scale_data_;
            }
            scale_data_ = other.scale_data_;
            other.scale_data_ = nullptr;
            scale_size_ = other.scale_size_;
        }
        return *this;
    }

    ~ScaleBitmapBuffer() {
        if (scale_data_ != nullptr) {
            delete []scale_data_;
            scale_data_ = nullptr;
        }
    }
};


class BitmapImpl {
public:
    BitmapImpl()
        :bitmap_(){}
    BitmapImpl(BitmapBuffer bitmap, float scale_factor)
        :bitmap_(bitmap){
         if (scale_factor != 1.0) {
             scale_factor_ = scale_factor;
             is_scale_ = true;
         } else {
             scale_factor_ = 1.0;
             is_scale_ = false;
         }
    }
    Error draw(const PixelDrawer& drawPixel, const Box<pxs>& bounds);
    Error GetBitmapColor(int x, int y, BYTE& r, BYTE& g, BYTE& b);
    Error ReadBmp();
    ScaleBitmapBuffer ScaleBmp();

    struct bitmap_info_header {
        DWORD     size;
        int32_t   width;
        int32_t   height;
        WORD      planes;
        WORD      bitCount;
        DWORD     compression;
        DWORD     sizeImage;
        int32_t   x_pels;
        int32_t   y_pels;
        DWORD     clrUsed;
        DWORD     clrImportant;
    };

private:
    bitmap_info_header bih = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    bitmap_info_header scale_bih = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    long LineByteWidth = 0;
    long ScaleLineByteWidth = 0;
    BitmapBuffer bitmap_;
    ScaleBitmapBuffer scale_bitmap_;
    float scale_factor_ = 1.0;
    bool is_scale_ = false;
    int32_t scale_width = 0;
    int32_t scale_height = 0;
};

/**
 * Bitmap is a LayoutElement and should be used as second argument in the BEGIN_ELEMENT() macro.
 * The template argument Derived is the new class derived from Bitmap, that is created by the
 * BEGIN_ELEMENT() macro.
 */
template <typename Derived> class Bitmap : public LayoutElement<Derived>, public BitmapImpl {
public:
    static const constexpr int bitmap = 0;
    static const constexpr float scale_factor = 1.0;
    Bitmap() = default;
    template <typename Context>
    Bitmap(const Context& context)
        : LayoutElement<Derived>(context),
          BitmapImpl(getBitmap(Derived::bitmap), Derived::scale_factor) {}

    Error draw(const PixelDrawer& drawPixel) {
        return BitmapImpl::draw(drawPixel, this->bounds_);
    }
};

}
#define BITMAP(name) TEEUI_BITMAP_##name()

#define DECLARE_BITMAP_BUFFER(name, buffer, ...)                                                     \
    struct TEEUI_BITMAP_##name {};                                                                   \
    inline BitmapBuffer getBitmap(TEEUI_BITMAP_##name) { return BitmapBuffer(buffer, ##__VA_ARGS__); }

#define Bitmap(buffer) static const constexpr auto bitmap = buffer

#define ScaleParam(factor) static const constexpr auto scale_factor = factor

#endif  // LIBTEEUI_BITMAP_H_
