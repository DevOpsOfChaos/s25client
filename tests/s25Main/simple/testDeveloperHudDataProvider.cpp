// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DeveloperHudDataProvider.h"
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace {
const DeveloperHudDataGroup* FindGroup(const DeveloperHudViewModel& data, const std::string& key)
{
    const auto groupIt = std::find_if(data.developmentExportGroups.begin(), data.developmentExportGroups.end(),
                                      [&key](const DeveloperHudDataGroup& group) { return group.key == key; });
    return groupIt == data.developmentExportGroups.end() ? nullptr : &*groupIt;
}

const DeveloperHudDataField* FindField(const DeveloperHudDataGroup& group, const std::string& key)
{
    const auto fieldIt = std::find_if(group.fields.begin(), group.fields.end(),
                                      [&key](const DeveloperHudDataField& field) { return field.key == key; });
    return fieldIt == group.fields.end() ? nullptr : &*fieldIt;
}
} // namespace

BOOST_AUTO_TEST_CASE(MockDeveloperHudDataProviderBuildsCompactOverlayData)
{
    const MockDeveloperHudDataProvider provider;
    const DeveloperHudViewModel data = provider.GetViewModel();

    BOOST_TEST(!data.isLiveData);
    BOOST_TEST(data.sourceLabel == "Static mock data");
    BOOST_TEST(data.totalSoldiers == 184u);
    BOOST_TEST(data.messageCount == 3u);
    BOOST_TEST(data.topBarChips[1] == "S 184");
    BOOST_TEST(data.topBarChips[2] == "Au 74");
    BOOST_TEST(data.topBarChips[3] == "Sw 38");
    BOOST_TEST(data.topBarChips[4] == "Msg 3");
    BOOST_TEST(data.resourceChips[0].label == "Gold");
    BOOST_TEST(data.resourceChips[1].label == "Swords");
    BOOST_TEST(data.resourceChips[2].label == "Food");
    BOOST_TEST(data.messageLane == "Message lane: 3 unread post messages");
    BOOST_TEST(data.selectedSummary.find("headquarters mock") != std::string::npos);
    BOOST_TEST(data.developmentExportSummary == "HUD export contract: 8 groups / 27 fields");
    BOOST_TEST_REQUIRE(FindGroup(data, "player"));
    BOOST_TEST_REQUIRE(FindGroup(data, "military"));
    BOOST_TEST_REQUIRE(FindGroup(data, "resources"));
    BOOST_TEST_REQUIRE(FindGroup(data, "messages"));
    BOOST_TEST_REQUIRE(FindGroup(data, "selection"));
    BOOST_TEST_REQUIRE(FindGroup(data, "map"));
    BOOST_TEST_REQUIRE(FindGroup(data, "commands"));
    BOOST_TEST_REQUIRE(FindGroup(data, "economy"));

    const DeveloperHudDataGroup* militaryGroup = FindGroup(data, "military");
    BOOST_TEST_REQUIRE(militaryGroup);
    const DeveloperHudDataField* militaryStatus = FindField(*militaryGroup, "military.status.label");
    BOOST_TEST_REQUIRE(militaryStatus);
    BOOST_TEST(militaryStatus->value == "Guarded");
    BOOST_TEST(static_cast<int>(militaryStatus->availability) == static_cast<int>(DeveloperHudFieldAvailability::Mock));

    const DeveloperHudDataGroup* commandGroup = FindGroup(data, "commands");
    BOOST_TEST_REQUIRE(commandGroup);
    const DeveloperHudDataField* commandDispatch = FindField(*commandGroup, "commands.dispatch");
    BOOST_TEST_REQUIRE(commandDispatch);
    BOOST_TEST(commandDispatch->value == "disabled / no commands");
    BOOST_TEST(static_cast<int>(commandDispatch->availability)
               == static_cast<int>(DeveloperHudFieldAvailability::Placeholder));
}

BOOST_AUTO_TEST_CASE(DeveloperHudViewModelFormatsLiveSnapshotReadOnlyFields)
{
    DeveloperHudSnapshot snapshot;
    snapshot.isLiveData = true;
    snapshot.sourceLabel = "Live read-only game data";
    snapshot.playerName = "Ada";
    snapshot.playerId = 1;
    snapshot.soldiersByRank = {{1, 2, 3, 4, 5}};
    snapshot.armoredSoldiers = 6;
    snapshot.gold = 7;
    snapshot.coins = 8;
    snapshot.swords = 9;
    snapshot.food = 10;
    snapshot.messageCount = 0;
    snapshot.mapSize = MapExtent(64, 48);
    snapshot.selectedPoint = MapPoint::Invalid();

    const DeveloperHudViewModel data = MakeDeveloperHudViewModel(snapshot);

    BOOST_TEST(data.isLiveData);
    BOOST_TEST(data.playerLabel == "P2 Ada");
    BOOST_TEST(data.totalSoldiers == 15u);
    BOOST_TEST(data.topBarChips[1] == "S 15");
    BOOST_TEST(data.topBarChips[2] == "Au 7");
    BOOST_TEST(data.topBarChips[3] == "Sw 9");
    BOOST_TEST(data.resourceChips[2].value == "10");
    BOOST_TEST(data.militarySummary == "Soldiers 15 | Ranks 1/2/3/4/5 | Armor 6");
    BOOST_TEST(data.messageLane == "Message lane: no unread post messages");
    BOOST_TEST(data.mapSummary == "Map 64x48 | read-only");
    BOOST_TEST(data.selectedSummary == "Selected: no map point");
    BOOST_TEST(data.developmentExportSummary == "HUD export contract: 8 groups / 27 fields");

    const DeveloperHudDataGroup* militaryGroup = FindGroup(data, "military");
    BOOST_TEST_REQUIRE(militaryGroup);
    const DeveloperHudDataField* capacity = FindField(*militaryGroup, "military.capacity.max");
    BOOST_TEST_REQUIRE(capacity);
    BOOST_TEST(static_cast<int>(capacity->availability)
               == static_cast<int>(DeveloperHudFieldAvailability::NotSafelyAccessible));
    BOOST_TEST(capacity->source == "future military capacity accessor");

    const DeveloperHudDataGroup* messageGroup = FindGroup(data, "messages");
    BOOST_TEST_REQUIRE(messageGroup);
    const DeveloperHudDataField* latestMessage = FindField(*messageGroup, "messages.latest.label");
    BOOST_TEST_REQUIRE(latestMessage);
    BOOST_TEST(static_cast<int>(latestMessage->availability)
               == static_cast<int>(DeveloperHudFieldAvailability::NotSafelyAccessible));
    BOOST_TEST(latestMessage->source == "future post message summary accessor");
}

BOOST_AUTO_TEST_CASE(DeveloperHudAvailabilityLabelsAreStableForDevelopmentExport)
{
    BOOST_TEST(ToString(DeveloperHudFieldAvailability::LiveReadOnly) == "live read-only");
    BOOST_TEST(ToString(DeveloperHudFieldAvailability::Mock) == "mock");
    BOOST_TEST(ToString(DeveloperHudFieldAvailability::Placeholder) == "placeholder");
    BOOST_TEST(ToString(DeveloperHudFieldAvailability::NotSafelyAccessible) == "not safely accessible yet");
}
