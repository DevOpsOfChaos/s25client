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

IntegerScaleViewport CalculateIntegerScaleViewport(Extent sourceSize, Extent targetSize);

} // namespace helpers
