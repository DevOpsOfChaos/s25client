// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Point.h"
#include "Rect.h"

namespace helpers {

struct IntegerScaleViewport
{
    unsigned scale;
    Rect viewport;
    bool fits;
};

struct IntegerScalePointMapping
{
    bool inside;
    Position point;
};

struct IntegerScaleRectMapping
{
    bool inside;
    Rect rect;
};

IntegerScaleViewport CalculateIntegerScaleViewport(Extent sourceSize, Extent targetSize);
bool IsTargetPointInsideIntegerScaleViewport(const IntegerScaleViewport& scaling, Position targetPoint);
IntegerScalePointMapping MapTargetPointToSourcePoint(const IntegerScaleViewport& scaling, Position targetPoint);
IntegerScalePointMapping MapSourcePointToTargetPoint(const IntegerScaleViewport& scaling, Extent sourceSize,
                                                     Position sourcePoint);
IntegerScaleRectMapping MapSourceRectToTargetRect(const IntegerScaleViewport& scaling, Extent sourceSize,
                                                  Rect sourceRect);

} // namespace helpers
