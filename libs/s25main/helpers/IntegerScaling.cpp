// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "helpers/IntegerScaling.h"
#include <algorithm>
#include <cstdint>
#include <limits>

namespace helpers {

namespace {

    constexpr int maxRectCoordinate = std::numeric_limits<int>::max();

    bool IsRepresentableRectValue(const uint64_t value) { return value <= static_cast<uint64_t>(maxRectCoordinate); }

    bool IsRepresentableRectSize(const Extent size)
    {
        return IsRepresentableRectValue(size.x) && IsRepresentableRectValue(size.y);
    }

    Rect MakeRect(const uint64_t left, const uint64_t top, const uint64_t width, const uint64_t height)
    {
        return Rect(Position(static_cast<int>(left), static_cast<int>(top)),
                    Extent(static_cast<unsigned>(width), static_cast<unsigned>(height)));
    }

    IntegerScaleViewport EmptyNonFittingViewport()
    {
        return IntegerScaleViewport{1u, Rect(Position(0, 0), Extent(0, 0)), false};
    }

    bool IsPointInsideRect(const Position point, const Rect rect)
    {
        return point.x >= rect.left && point.y >= rect.top && point.x < rect.right && point.y < rect.bottom;
    }

    bool IsValidSourceSizeForViewport(const IntegerScaleViewport& scaling, const Extent sourceSize)
    {
        if(!scaling.fits || scaling.scale == 0u || sourceSize.x == 0u || sourceSize.y == 0u)
            return false;

        const uint64_t width = static_cast<uint64_t>(sourceSize.x) * scaling.scale;
        const uint64_t height = static_cast<uint64_t>(sourceSize.y) * scaling.scale;
        return IsRepresentableRectValue(width) && IsRepresentableRectValue(height)
               && scaling.viewport.getSize() == Extent(static_cast<unsigned>(width), static_cast<unsigned>(height));
    }

    bool IsRectInsideSourceSize(const Rect rect, const Extent sourceSize)
    {
        return rect.left >= 0 && rect.top >= 0 && rect.left <= rect.right && rect.top <= rect.bottom
               && static_cast<uint64_t>(rect.right) <= sourceSize.x
               && static_cast<uint64_t>(rect.bottom) <= sourceSize.y;
    }

} // namespace

IntegerScaleViewport CalculateIntegerScaleViewport(const Extent sourceSize, const Extent targetSize)
{
    if(sourceSize.x == 0u || sourceSize.y == 0u || targetSize.x == 0u || targetSize.y == 0u)
        return EmptyNonFittingViewport();

    if(!IsRepresentableRectSize(targetSize))
        return EmptyNonFittingViewport();

    const unsigned scaleX = targetSize.x / sourceSize.x;
    const unsigned scaleY = targetSize.y / sourceSize.y;
    const unsigned scale = std::min(scaleX, scaleY);

    if(scale == 0u)
        return IntegerScaleViewport{1u, Rect(Position(0, 0), targetSize), false};

    const uint64_t viewportWidth = static_cast<uint64_t>(sourceSize.x) * scale;
    const uint64_t viewportHeight = static_cast<uint64_t>(sourceSize.y) * scale;
    const uint64_t originX = (static_cast<uint64_t>(targetSize.x) - viewportWidth) / 2u;
    const uint64_t originY = (static_cast<uint64_t>(targetSize.y) - viewportHeight) / 2u;

    if(!IsRepresentableRectValue(viewportWidth) || !IsRepresentableRectValue(viewportHeight)
       || !IsRepresentableRectValue(originX + viewportWidth) || !IsRepresentableRectValue(originY + viewportHeight))
        return EmptyNonFittingViewport();

    return IntegerScaleViewport{scale, MakeRect(originX, originY, viewportWidth, viewportHeight), true};
}

bool IsTargetPointInsideIntegerScaleViewport(const IntegerScaleViewport& scaling, const Position targetPoint)
{
    return scaling.fits && scaling.scale != 0u && IsPointInsideRect(targetPoint, scaling.viewport);
}

IntegerScalePointMapping MapTargetPointToSourcePoint(const IntegerScaleViewport& scaling, const Position targetPoint)
{
    if(!IsTargetPointInsideIntegerScaleViewport(scaling, targetPoint))
        return IntegerScalePointMapping{false, Position(0, 0)};

    return IntegerScalePointMapping{true,
                                    Position((targetPoint.x - scaling.viewport.left) / static_cast<int>(scaling.scale),
                                             (targetPoint.y - scaling.viewport.top) / static_cast<int>(scaling.scale))};
}

IntegerScalePointMapping MapSourcePointToTargetPoint(const IntegerScaleViewport& scaling, const Extent sourceSize,
                                                     const Position sourcePoint)
{
    if(!IsValidSourceSizeForViewport(scaling, sourceSize)
       || !IsPointInsideRect(sourcePoint, Rect(Position(0, 0), sourceSize)))
        return IntegerScalePointMapping{false, Position(0, 0)};

    return IntegerScalePointMapping{true,
                                    Position(scaling.viewport.left + sourcePoint.x * static_cast<int>(scaling.scale),
                                             scaling.viewport.top + sourcePoint.y * static_cast<int>(scaling.scale))};
}

IntegerScaleRectMapping MapSourceRectToTargetRect(const IntegerScaleViewport& scaling, const Extent sourceSize,
                                                  const Rect sourceRect)
{
    if(!IsValidSourceSizeForViewport(scaling, sourceSize) || !IsRectInsideSourceSize(sourceRect, sourceSize))
        return IntegerScaleRectMapping{false, Rect(Position(0, 0), Extent(0, 0))};

    const uint64_t left =
      static_cast<uint64_t>(scaling.viewport.left) + static_cast<uint64_t>(sourceRect.left) * scaling.scale;
    const uint64_t top =
      static_cast<uint64_t>(scaling.viewport.top) + static_cast<uint64_t>(sourceRect.top) * scaling.scale;
    const uint64_t width = static_cast<uint64_t>(sourceRect.getSize().x) * scaling.scale;
    const uint64_t height = static_cast<uint64_t>(sourceRect.getSize().y) * scaling.scale;

    if(!IsRepresentableRectValue(left + width) || !IsRepresentableRectValue(top + height))
        return IntegerScaleRectMapping{false, Rect(Position(0, 0), Extent(0, 0))};

    return IntegerScaleRectMapping{true, MakeRect(left, top, width, height)};
}

} // namespace helpers
