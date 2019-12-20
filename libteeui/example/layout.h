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

#ifndef TEEUI_LIBTEEUI_EXAMPLE_LAYOUT_H_
#define TEEUI_LIBTEEUI_EXAMPLE_LAYOUT_H_

#include <teeui/button.h>
#include <teeui/label.h>
#include <teeui/utils.h>

#include "fonts.h"

namespace teeui {

DECLARE_PARAMETER(RightEdgeOfScreen);
DECLARE_PARAMETER(BottomOfScreen);
DECLARE_PARAMETER(PowerButtonTop);
DECLARE_PARAMETER(PowerButtonBottom);
DECLARE_PARAMETER(VolUpButtonTop);
DECLARE_PARAMETER(VolUpButtonBottom);
DECLARE_PARAMETER(DefaultFontSize);  // 14_dp regular and 18_dp magnified
DECLARE_PARAMETER(BodyFontSize);     // 16_dp regular and 20_dp magnified

NEW_PARAMETER_SET(ConUIParameters, RightEdgeOfScreen, BottomOfScreen, PowerButtonTop,
                  PowerButtonBottom, VolUpButtonTop, VolUpButtonBottom, DefaultFontSize,
                  BodyFontSize);

CONSTANT(BorderWidth, 24_dp);
CONSTANT(PowerButtonCenter, (PowerButtonTop() + PowerButtonBottom()) / 2_px);
CONSTANT(VolUpButtonCenter, (VolUpButtonTop() + VolUpButtonBottom()) / 2.0_px);
CONSTANT(GrayZone, 12_dp);
CONSTANT(RightLabelEdge, RightEdgeOfScreen() - BorderWidth - GrayZone);
CONSTANT(LabelWidth, RightLabelEdge - BorderWidth);
CONSTANT(DefaultTextColor, Color(0xff212121));

CONSTANT(SQRT2, 1.4142135623_dp);
CONSTANT(SQRT8, 2.828427125_dp);

CONSTANT(ARROW_SHAPE,
         CONVEX_OBJECTS(CONVEX_OBJECT(Vec2d{.0_dp, .0_dp}, Vec2d{6.0_dp, 6.0_dp},
                                      Vec2d{6.0_dp - SQRT8, 6.0_dp}, Vec2d{-SQRT2, SQRT2}),
                        CONVEX_OBJECT(Vec2d{6.0_dp - SQRT8, 6.0_dp}, Vec2d{6.0_dp, 6.0_dp},
                                      Vec2d{0.0_dp, 12.0_dp}, Vec2d{-SQRT2, 12.0_dp - SQRT2})));

DECLARE_FONT_BUFFER(RobotoRegular, RobotoRegular, RobotoRegular_length);
DECLARE_FONT_BUFFER(Shield, Shield, Shield_length);
CONSTANT(DefaultFont, FONT(RobotoRegular));

BEGIN_ELEMENT(LabelOK, teeui::Label)
FontSize(DefaultFontSize());
LineHeight(20_dp);
NumberOfLines(2);
Dimension(LabelWidth, HeightFromLines);
Position(BorderWidth, PowerButtonCenter - dim_h / 2.0_px);
DefaultText("Wiggle your big toe to confirm");
RightJustified;
VerticallyCentered;
TextColor(DefaultTextColor);
Font(DefaultFont);
END_ELEMENT();

BEGIN_ELEMENT(IconPower, teeui::Button, ConvexObjectCount(2))
Dimension(BorderWidth, PowerButtonBottom() - PowerButtonTop());
Position(RightEdgeOfScreen() - BorderWidth, PowerButtonTop());
CornerRadius(3_dp);
ButtonColor(0xff212121);
RoundTopLeft;
RoundBottomLeft;
ConvexObjectColor(0xffffffff);
ConvexObjects(ARROW_SHAPE);
END_ELEMENT();

BEGIN_ELEMENT(LabelCancel, teeui::Label)
FontSize(DefaultFontSize());
LineHeight(20_dp);
NumberOfLines(2);
Dimension(LabelWidth, HeightFromLines);
Position(BorderWidth, VolUpButtonCenter - dim_h / 2.0_px);
DefaultText("Wink left then right thrice to cancel");
RightJustified;
VerticallyCentered;
TextColor(DefaultTextColor);
Font(DefaultFont);
END_ELEMENT();

BEGIN_ELEMENT(IconVolUp, teeui::Button, ConvexObjectCount(2))
Dimension(BorderWidth, VolUpButtonBottom() - VolUpButtonTop());
Position(RightEdgeOfScreen() - BorderWidth, VolUpButtonTop());
CornerRadius(5_dp);
ButtonColor(0xffffffff);
ConvexObjectColor(0xff212121);
ConvexObjects(ARROW_SHAPE);
END_ELEMENT();

BEGIN_ELEMENT(IconShield, teeui::Label)
FontSize(36_dp);
LineHeight(36_dp);
NumberOfLines(1);
Dimension(LabelWidth, HeightFromLines);
Position(BorderWidth, BOTTOM_EDGE_OF(LabelCancel) + 60_dp);
DefaultText("A");  // ShieldTTF has just one glyph at the code point for capital A
TextColor(0xfff48542);
Font(FONT(Shield));
END_ELEMENT();

BEGIN_ELEMENT(LabelTitle, teeui::Label)
FontSize(22_dp);
LineHeight(28_dp);
NumberOfLines(1);
Dimension(RightEdgeOfScreen() - BorderWidth, HeightFromLines);
Position(BorderWidth, BOTTOM_EDGE_OF(IconShield) + 16_dp);
DefaultText("Android Protected Confirmation");
Font(DefaultFont);
VerticallyCentered;
TextColor(DefaultTextColor);
END_ELEMENT();

BEGIN_ELEMENT(LabelHint, teeui::Label)
FontSize(DefaultFontSize());
LineHeight(DefaultFontSize() * 1.5_px);
NumberOfLines(4);
Dimension(LabelWidth, HeightFromLines);
Position(BorderWidth, BottomOfScreen() - BorderWidth - dim_h);
DefaultText("This confirmation provides an extra layer of security for the action that you're "
            "about to take");
VerticallyCentered;
TextColor(DefaultTextColor);
Font(DefaultFont);
END_ELEMENT();

BEGIN_ELEMENT(LabelBody, teeui::Label)
FontSize(BodyFontSize());
LineHeight(BodyFontSize() * 1.4_px);
NumberOfLines(20);
Position(BorderWidth, BOTTOM_EDGE_OF(LabelTitle) + 24_dp);
Dimension(LabelWidth, LabelHint::pos_y - pos_y - 24_dp);
DefaultText(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
    "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
    "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
    "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
TextColor(DefaultTextColor);
Font(DefaultFont);
END_ELEMENT();

NEW_LAYOUT(ConfUILayout, LabelOK, IconPower, LabelCancel, IconVolUp, IconShield, LabelTitle,
           LabelHint, LabelBody);

}  // namespace teeui

#endif  // TEEUI_LIBTEEUI_EXAMPLE_LAYOUT_H_
