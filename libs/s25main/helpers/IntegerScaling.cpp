// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "helpers/IntegerScaling.h"
#include <algorithm>

namespace helpers {

IntegerScaleViewport CalculateIntegerScaleViewport(const Extent sourceSize, const Extent targetSize)
{
    if(sourceSize.x == 0u || sourceSize.y == 0u || targetSize.x == 0u || targetSize.y == 0u)
        return IntegerScaleViewport{1u, Rect(Position(0, 0), Extent(0, 0)), false};

    const unsigned scaleX = targetSize.x / sourceSize.x;
    const unsigned scaleY = targetSize.y / sourceSize.y;
    const unsigned scale = std::min(scaleX, scaleY);

    if(scale == 0u)
        return IntegerScaleViewport{1u, Rect(Position(0, 0), targetSize), false};

    const Extent viewportSize = sourceSize * scale;
    const Position origin((targetSize.x - viewportSize.x) / 2u, (targetSize.y - viewportSize.y) / 2u);
    return IntegerScaleViewport{scale, Rect(origin, viewportSize), true};
}

} // namespace helpers
