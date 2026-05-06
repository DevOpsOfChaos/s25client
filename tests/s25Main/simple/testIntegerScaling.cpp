// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PointOutput.h"
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

BOOST_AUTO_TEST_CASE(TooSmallTargetFallsBackDeterministically)
{
    const auto result = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(640, 480));

    BOOST_TEST(result.scale == 1u);
    BOOST_TEST(!result.fits);
    BOOST_TEST(result.viewport.left == 0);
    BOOST_TEST(result.viewport.top == 0);
    BOOST_TEST(result.viewport.getSize() == Extent(640, 480));
}

BOOST_AUTO_TEST_CASE(ZeroDimensionsAreDefensive)
{
    const auto zeroSourceWidth = helpers::CalculateIntegerScaleViewport(Extent(0, 600), Extent(800, 600));
    BOOST_TEST(zeroSourceWidth.scale == 1u);
    BOOST_TEST(!zeroSourceWidth.fits);
    BOOST_TEST(zeroSourceWidth.viewport.getSize() == Extent(0, 0));

    const auto zeroTargetHeight = helpers::CalculateIntegerScaleViewport(Extent(800, 600), Extent(800, 0));
    BOOST_TEST(zeroTargetHeight.scale == 1u);
    BOOST_TEST(!zeroTargetHeight.fits);
    BOOST_TEST(zeroTargetHeight.viewport.getSize() == Extent(0, 0));
}

BOOST_AUTO_TEST_SUITE_END()
