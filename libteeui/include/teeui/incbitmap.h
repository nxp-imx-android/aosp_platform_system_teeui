/*
 *
 * Copyright 2022, The Android Open Source Project
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

#ifndef LIBTEEUI_INCLUDE_TEEUI_INCBITMAP_H_
#define LIBTEEUI_INCLUDE_TEEUI_INCBITMAP_H_

#ifdef __ASSEMBLY__
.macro TEEUI_ASM_INCBITMAP bitmapname, filename
.section .rodata
.align 4
.globl \bitmapname
\bitmapname:
.incbin "\filename"
.section .rodata
.align 1
\bitmapname\()_end:
.section .rodata
.align 4;
.globl \bitmapname\()_length
\bitmapname\()_length:
.long \bitmapname\()_end - \bitmapname - 1
.endmacro

#define TEEUI_INCBITMAP(bitmapname, filename) TEEUI_ASM_INCBITMAP bitmapname, filename
#else
#define TEEUI_INCBITMAP(bitmapname, ...)  \
    extern unsigned char bitmapname[];    \
    extern unsigned int bitmapname##_length
#endif

#endif  // LIBTEEUI_INCLUDE_TEEUI_INCBITMAP_H_
