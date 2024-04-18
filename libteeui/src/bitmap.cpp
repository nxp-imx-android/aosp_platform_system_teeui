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

#include <teeui/bitmap.h>

#define WIDTHSIZE(bits) (((bits)+31)/32*4)

namespace teeui {

Error BitmapImpl::draw(const PixelDrawer& drawPixel, const Box<pxs>& bounds) {
    Error error;
    error = ReadBmp();
    bitmap_info_header g_bih;
    if (is_scale_) {
        scale_bitmap_ = ScaleBmp();
        g_bih = scale_bih;
        if (error != Error::OK) {
            TEEUI_LOG << "ScaleBitmap Failed " << error << ENDL;
            return error;
        }
    } else {
        g_bih = bih;
    }

    auto drawBitmap = [&, this](uint32_t x, uint32_t y) -> Error {
        BYTE R, G, B;
        Error error = GetBitmapColor(x, y, R, G, B);
        if (error != Error::OK) {
            TEEUI_LOG << "GetDIBColor Failed " << error << ENDL;
            return error;
        }
        Color color = (0xff << 24) | (B << 16) | (G << 8) | (R);
        return drawPixel((uint32_t)(bounds.x().count()) + x,
               (uint32_t)(bounds.y().count()) + y, (color & 0xffffffff));
    };

#ifdef DRAW_DEBUG_MARKERS
    auto drawBox = [&](const Box<pxs>& box, Color c) {
        for (int y = 0; y < box.h().count(); ++y) {
            for (int x = 0; x < box.w().count(); ++x) {
                drawPixel(box.x().count() + x, box.y().count() + y, (c & 0xffffff) | 0x40000000);
            }
        }
    };

    drawBox(bounds, 0xff);
#endif

    for (int32_t i = 0; i < g_bih.width; i++) {
        for (int32_t j = 0; j < g_bih.height; j++) {
            error = drawBitmap(i, j);
            if (error != Error::OK) {
                TEEUI_LOG << "drawBitmap failed " << error << ENDL;
                return error;
            }
        }
    }

    return error;
}

Error BitmapImpl::GetBitmapColor(int x, int y, BYTE& r, BYTE& g, BYTE& b) {
    int px_byte_count;
    BYTE* ptr;
    bitmap_info_header bih_temp;
    bih_temp = is_scale_ == true ? scale_bih: bih;

    if (x < 0 || x >= bih_temp.width || y < 0 || y >= bih_temp.height)
    {
        TEEUI_LOG << "BitmapOutofRnage" << "x= " << x << "," << "y= " << y << ENDL;
        return Error::BitmapOutofRnage;
    }
    px_byte_count = bih_temp.bitCount / 8;
    if (is_scale_)
        ptr = (BYTE*)scale_bitmap_.scale_data_ +
               x * px_byte_count + (bih_temp.height - 1 - y) * ScaleLineByteWidth;
    else
        ptr = (BYTE*)(bitmap_.data() + 7 * sizeof(WORD) + sizeof(bitmap_info_header)) +
               x * px_byte_count + (bih_temp.height - 1 - y) * LineByteWidth;

    b = *ptr;
    g = *(ptr + 1);
    r = *(ptr + 2);
    return Error::OK;
}

Error BitmapImpl::ReadBmp() {

    WORD bfh[7];
    long px_byte_count;

    /* parse bitmap file header */
    memcpy(bfh, bitmap_.data(), 7 * sizeof(WORD));
    WORD head;
    memcpy(&head, bfh, sizeof(WORD));
    if (head != (WORD)(((WORD)'B') | ('M' << 8)))
    {
        TEEUI_LOG << "it is not bitmap format" << ENDL;
        return Error::BitmapReadFailed;
    }

    /* parse bitmap header*/
    memcpy(&bih, bitmap_.data() +  7 * sizeof(WORD), sizeof(bitmap_info_header));
    if (bih.bitCount < 24)
    {
        TEEUI_LOG << "the bitmap header is not correct" << ENDL;
        return Error::BitmapReadFailed;
    }

    px_byte_count = bih.bitCount / 8;
    LineByteWidth = bih.width * (px_byte_count);
    if ((LineByteWidth % 4) != 0)
        LineByteWidth += 4 - (LineByteWidth % 4);

    return Error::OK;
}

ScaleBitmapBuffer BitmapImpl::ScaleBmp() {
    scale_bih = bih;

    scale_width = scale_factor_ * bih.width;
    scale_height = scale_factor_ * bih.height;

    scale_bih.height = scale_height;//scale bmp height
    scale_bih.width = scale_width;  //scale bmp width

    int scale_dst_bitmapwidth = WIDTHSIZE(scale_bih.width * scale_bih.bitCount);//width must be multi 4bytes
    scale_bih.sizeImage = scale_dst_bitmapwidth * scale_bih.height;// new buffer size

    int width = bih.width;
    int height = bih.height;

    //allocate buffer
    int origin_width_align = WIDTHSIZE(width * bih.bitCount);//src width
    int scale_width_align = WIDTHSIZE(scale_bih.width * scale_bih.bitCount);//dst width
    ScaleLineByteWidth = scale_width_align;

    BYTE* scaleDataMid = new BYTE[scale_dst_bitmapwidth * scale_height];// allocate new buffer

    memset(scaleDataMid, 0, scale_dst_bitmapwidth * scale_height);
    const BYTE* bitmapBuffer = bitmap_.data() +  7 * sizeof(WORD) + sizeof(bitmap_info_header);

    for (int h = 0; h < scale_height; h++) {
        for (int w = 0; w < scale_width; w++)
        {
            double decimal_h = h * height / (double)scale_height;
            double decimal_w = w * width / (double)scale_width;
            int integer_h = decimal_h;
            int integer_w = decimal_w;
            double dis_to_ax = decimal_w - integer_w;
            double dis_to_ay = decimal_h - integer_h;

            int left_top = integer_h * origin_width_align + integer_w * 3;
            int right_top = integer_h * origin_width_align + (integer_w + 1) * 3;
            int left_bottom = (integer_h + 1) * origin_width_align + integer_w * 3;
            int right_bottom = (integer_h + 1) * origin_width_align + (integer_w + 1) * 3;
            if (integer_h + 1 >= width)
            {
                left_bottom = left_top;
                right_bottom = right_top;
            }
            if (integer_w + 1 >= height)
            {
                right_top = left_top;
                right_bottom = left_bottom;
            }

            int pixel_point = h * scale_width_align + w * 3;
            for (int i = 0; i < 3; i++)
            {
                scaleDataMid[pixel_point + i] =
                    bitmapBuffer[left_top + i] * (1 - dis_to_ax) * (1 - dis_to_ay) +
                    bitmapBuffer[right_top + i] * dis_to_ax * (1 - dis_to_ay) +
                    bitmapBuffer[left_bottom + i] * dis_to_ay * (1 - dis_to_ax) +
                    bitmapBuffer[right_bottom + i] * dis_to_ay * dis_to_ax;
            }

        }
    }
    return ScaleBitmapBuffer(scaleDataMid, scale_dst_bitmapwidth * scale_height);
}

}
