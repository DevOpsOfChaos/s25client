// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PointOutput.h"
#include "RectOutput.h"
#include "helpers/IntegerScaling.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(IntegerScalingSuite)

BOOST_AUTO_TEST_CASE(ExactOneX)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(800, 600));

    BOOST_TEST(result.scale == 1u);
    BOOST_TEST(result.fits);
    BOOST_TEST(result.viewport.left == 0);
    BOOST_TEST(result.viewport.top == 0);
    BOOST_TEST(result.viewport.right == 800);
    BOOST_TEST(result.viewport.bottom == 600);
}

BOOST_AUTO_TEST_CASE(TwoAndThreeXFit)
{
    const auto result2x = helpers::CalculateIntegerScaleViewport(Extent(640, 360), Extent(1280, 720));
    BOOST_TEST(result2x.scale == 2u);
    BOOST_TEST(result2x.fits);
    BOOST_TEST(result2x.viewport.getSize() == Extent(1280, 720));

    const auto result3x = helpers::CalculateIntegerScaleViewport(Extent(640, 360), Extent(1920, 1080));
    BOOST_TEST(result3x.scale == 3u);
    BOOST_TEST(result3x.fits);
    BOOST_TEST(result3x.viewport.getSize() == Extent(1920, 1080));
}

BOOST_AUTO_TEST_CASE(NonIntegerTargetChoosesLowerInteger)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(640, 360), Extent(1919, 1079));

    BOOST_TEST(result.scale == 2u);
    BOOST_TEST(result.fits);
    BOOST_TEST(result.viewport.getSize() == Extent(1280, 720));
}

BOOST_AUTO_TEST_CASE(ViewportIsCentered)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(640, 360), Extent(1600, 1000));

    BOOST_TEST(result.scale == 2u);
    BOOST_TEST(result.fits);
    BOOST_TEST(result.viewport.left == 160);
    BOOST_TEST(result.viewport.top == 140);
    BOOST_TEST(result.viewport.right == 1440);
    BOOST_TEST(result.viewport.bottom == 860);
}

BOOST_AUTO_TEST_CASE(ViewportIsCenteredWithOddLeftover)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(640, 360), Extent(1919, 1079));

    BOOST_TEST(result.scale == 2u);
    BOOST_TEST(result.fits);
    BOOST_TEST(result.viewport == Rect(319, 179, 1280, 720));
}

BOOST_AUTO_TEST_CASE(TooSmallTargetFallsBackDeterministically)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(640, 480));

    BOOST_TEST(result.scale == 1u);
    BOOST_TEST(!result.fits);
    BOOST_TEST(result.viewport.left == 0);
    BOOST_TEST(result.viewport.top == 0);
    BOOST_TEST(result.viewport.getSize() == Extent(640, 480));
}

BOOST_AUTO_TEST_CASE(ZeroSourceDimensionsAreDefensive)
{
    const auto zeroSourceWidth = helpers::CalculateIntegerScaleViewport(Extent(0, 600), Extent(800, 600));
    BOOST_TEST(zeroSourceWidth.scale == 1u);
    BOOST_TEST(!zeroSourceWidth.fits);
    BOOST_TEST(zeroSourceWidth.viewport.getSize() == Extent(0, 0));

    const auto zeroSourceHeight = helpers::CalculateIntegerScaleViewport(Extent(800, 0), Extent(800, 600));
    BOOST_TEST(zeroSourceHeight.scale == 1u);
    BOOST_TEST(!zeroSourceHeight.fits);
    BOOST_TEST(zeroSourceHeight.viewport.getSize() == Extent(0, 0));
}

BOOST_AUTO_TEST_CASE(ZeroTargetDimensionsAreDefensive)
{
    const auto zeroTargetWidth = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(0, 600));
    BOOST_TEST(zeroTargetWidth.scale == 1u);
    BOOST_TEST(!zeroTargetWidth.fits);
    BOOST_TEST(zeroTargetWidth.viewport.getSize() == Extent(0, 0));

    const auto zeroTargetHeight = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(800, 0));
    BOOST_TEST(zeroTargetHeight.scale == 1u);
    BOOST_TEST(!zeroTargetHeight.fits);
    BOOST_TEST(zeroTargetHeight.viewport.getSize() == Extent(0, 0));
}

BOOST_AUTO_TEST_CASE(TargetPointInsideViewportMapsToSourcePoint)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(320, 200), Extent(1000, 700));
    BOOST_TEST_REQUIRE(scaling.scale == 3u);
    BOOST_TEST_REQUIRE(scaling.viewport == Rect(20, 50, 960, 600));

    const auto mapping = helpers::MapTargetPointToSourcePoint(scaling, Position(23, 56));

    BOOST_TEST(mapping.inside);
    BOOST_TEST(mapping.point == Position(1, 2));
}

BOOST_AUTO_TEST_CASE(TargetPointBoundaryBehaviorIsHalfOpen)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(320, 200), Extent(1000, 700));

    BOOST_TEST(helpers::IsTargetPointInsideIntegerScaleViewport(scaling, Position(20, 50)));
    BOOST_TEST(helpers::MapTargetPointToSourcePoint(scaling, Position(20, 50)).point == Position(0, 0));
    BOOST_TEST(helpers::IsTargetPointInsideIntegerScaleViewport(scaling, Position(979, 649)));
    BOOST_TEST(helpers::MapTargetPointToSourcePoint(scaling, Position(979, 649)).point == Position(319, 199));
    BOOST_TEST(!helpers::IsTargetPointInsideIntegerScaleViewport(scaling, Position(980, 649)));
    BOOST_TEST(!helpers::IsTargetPointInsideIntegerScaleViewport(scaling, Position(979, 650)));
}

BOOST_AUTO_TEST_CASE(TargetPointOutsideViewportIsRejected)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(320, 200), Extent(1000, 700));

    const auto leftLetterbox = helpers::MapTargetPointToSourcePoint(scaling, Position(19, 50));
    const auto topLetterbox = helpers::MapTargetPointToSourcePoint(scaling, Position(20, 49));

    BOOST_TEST(!leftLetterbox.inside);
    BOOST_TEST(leftLetterbox.point == Position(0, 0));
    BOOST_TEST(!topLetterbox.inside);
    BOOST_TEST(topLetterbox.point == Position(0, 0));
}

BOOST_AUTO_TEST_CASE(TooSmallFallbackRejectsPointMapping)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(640, 480));

    BOOST_TEST(!helpers::IsTargetPointInsideIntegerScaleViewport(scaling, Position(10, 10)));
    BOOST_TEST(!helpers::MapTargetPointToSourcePoint(scaling, Position(10, 10)).inside);
    BOOST_TEST(!helpers::MapSourcePointToTargetPoint(scaling, Extent(800, 600), Position(10, 10)).inside);
}

BOOST_AUTO_TEST_CASE(SourcePointMapsToTargetPointWithSameOriginAndScale)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(320, 200), Extent(1000, 700));

    const auto mapping = helpers::MapSourcePointToTargetPoint(scaling, Extent(320, 200), Position(1, 2));
    const auto boundary = helpers::MapSourcePointToTargetPoint(scaling, Extent(320, 200), Position(320, 199));

    BOOST_TEST(mapping.inside);
    BOOST_TEST(mapping.point == Position(23, 56));
    BOOST_TEST(!boundary.inside);
    BOOST_TEST(boundary.point == Position(0, 0));
}

BOOST_AUTO_TEST_CASE(SourceRectMapsToTargetRectWithSameOriginAndScale)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(320, 200), Extent(1000, 700));

    const auto mapping = helpers::MapSourceRectToTargetRect(scaling, Extent(320, 200), Rect(10, 20, 30, 40));
    const auto outside = helpers::MapSourceRectToTargetRect(scaling, Extent(320, 200), Rect(300, 180, 30, 40));

    BOOST_TEST(mapping.inside);
    BOOST_TEST(mapping.rect == Rect(50, 110, 90, 120));
    BOOST_TEST(!outside.inside);
    BOOST_TEST(outside.rect == Rect(0, 0, 0, 0));
}

BOOST_AUTO_TEST_CASE(ReasonableLargeDimensionsAvoidOverflowProneMath)
{
    const auto scaling = helpers::CalculateIntegerScaleViewport(Extent(16000, 9000), Extent(64000, 40000));

    BOOST_TEST(scaling.scale == 4u);
    BOOST_TEST(scaling.fits);
    BOOST_TEST(scaling.viewport == Rect(0, 2000, 64000, 36000));

    const auto point = helpers::MapSourcePointToTargetPoint(scaling, Extent(16000, 9000), Position(15999, 8999));
    BOOST_TEST(point.inside);
    BOOST_TEST(point.point == Position(63996, 37996));
}

BOOST_AUTO_TEST_CASE(PresentationPlanNoScalingNeeded)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(800, 600), Extent(800, 600));

    BOOST_TEST(plan.sourceSize == Extent(800, 600));
    BOOST_TEST(plan.targetSize == Extent(800, 600));
    BOOST_TEST(plan.mappingActive);
    BOOST_TEST(plan.integerViewport.scale == 1u);
    BOOST_TEST(plan.integerViewport.viewport == Rect(0, 0, 800, 600));
    BOOST_TEST(plan.letterboxMargins.left == 0u);
    BOOST_TEST(plan.letterboxMargins.top == 0u);
    BOOST_TEST(plan.letterboxMargins.right == 0u);
    BOOST_TEST(plan.letterboxMargins.bottom == 0u);
}

BOOST_AUTO_TEST_CASE(PresentationPlanTwoAndThreeXIntegerPresentation)
{
    const auto plan2x = helpers::CalculateIntegerPresentationPlan(Extent(640, 360), Extent(1280, 720));
    BOOST_TEST(plan2x.mappingActive);
    BOOST_TEST(plan2x.integerViewport.scale == 2u);
    BOOST_TEST(plan2x.integerViewport.viewport == Rect(0, 0, 1280, 720));

    const auto plan3x = helpers::CalculateIntegerPresentationPlan(Extent(640, 360), Extent(1920, 1080));
    BOOST_TEST(plan3x.mappingActive);
    BOOST_TEST(plan3x.integerViewport.scale == 3u);
    BOOST_TEST(plan3x.integerViewport.viewport == Rect(0, 0, 1920, 1080));
}

BOOST_AUTO_TEST_CASE(PresentationPlanReportsLetterboxMargins)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(320, 200), Extent(1000, 700));

    BOOST_TEST(plan.mappingActive);
    BOOST_TEST(plan.integerViewport.scale == 3u);
    BOOST_TEST(plan.integerViewport.viewport == Rect(20, 50, 960, 600));
    BOOST_TEST(plan.letterboxMargins.left == 20u);
    BOOST_TEST(plan.letterboxMargins.top == 50u);
    BOOST_TEST(plan.letterboxMargins.right == 20u);
    BOOST_TEST(plan.letterboxMargins.bottom == 50u);
}

BOOST_AUTO_TEST_CASE(PresentationPlanDelegatesTargetPointMapping)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(320, 200), Extent(1000, 700));

    const auto mapping = helpers::MapPresentationTargetPointToSourcePoint(plan, Position(23, 56));

    BOOST_TEST(mapping.inside);
    BOOST_TEST(mapping.point == Position(1, 2));
}

BOOST_AUTO_TEST_CASE(PresentationPlanRejectsOutsideLetterboxPoint)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(320, 200), Extent(1000, 700));

    const auto leftLetterbox = helpers::MapPresentationTargetPointToSourcePoint(plan, Position(19, 50));
    const auto topLetterbox = helpers::MapPresentationTargetPointToSourcePoint(plan, Position(20, 49));

    BOOST_TEST(!leftLetterbox.inside);
    BOOST_TEST(leftLetterbox.point == Position(0, 0));
    BOOST_TEST(!topLetterbox.inside);
    BOOST_TEST(topLetterbox.point == Position(0, 0));
}

BOOST_AUTO_TEST_CASE(PresentationPlanHandlesGuiScaleLikeTargetSize)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(640, 360), Extent(1280, 720));

    BOOST_TEST(plan.targetSize == Extent(1280, 720));
    BOOST_TEST(plan.mappingActive);
    BOOST_TEST(plan.integerViewport.scale == 2u);
    BOOST_TEST(plan.integerViewport.viewport == Rect(0, 0, 1280, 720));
    BOOST_TEST(helpers::MapPresentationTargetPointToSourcePoint(plan, Position(1279, 719)).point == Position(639, 359));
}

BOOST_AUTO_TEST_CASE(PresentationPlanTinyWindowFallbackDisablesMapping)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(800, 600), Extent(640, 480));

    BOOST_TEST(!plan.mappingActive);
    BOOST_TEST(!plan.integerViewport.fits);
    BOOST_TEST(plan.integerViewport.scale == 1u);
    BOOST_TEST(plan.integerViewport.viewport == Rect(0, 0, 640, 480));
    BOOST_TEST(plan.letterboxMargins.left == 0u);
    BOOST_TEST(plan.letterboxMargins.top == 0u);
    BOOST_TEST(plan.letterboxMargins.right == 0u);
    BOOST_TEST(plan.letterboxMargins.bottom == 0u);
    BOOST_TEST(!helpers::MapPresentationTargetPointToSourcePoint(plan, Position(10, 10)).inside);
}

BOOST_AUTO_TEST_CASE(PresentationPlanOddLeftoversAreDeterministic)
{
    const auto plan = helpers::CalculateIntegerPresentationPlan(Extent(640, 360), Extent(1919, 1079));

    BOOST_TEST(plan.mappingActive);
    BOOST_TEST(plan.integerViewport.scale == 2u);
    BOOST_TEST(plan.integerViewport.viewport == Rect(319, 179, 1280, 720));
    BOOST_TEST(plan.letterboxMargins.left == 319u);
    BOOST_TEST(plan.letterboxMargins.top == 179u);
    BOOST_TEST(plan.letterboxMargins.right == 320u);
    BOOST_TEST(plan.letterboxMargins.bottom == 180u);
}

BOOST_AUTO_TEST_SUITE_END()
