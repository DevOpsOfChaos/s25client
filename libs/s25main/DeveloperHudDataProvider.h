// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "gameTypes/JobTypes.h"
#include "gameTypes/MapCoordinates.h"
#include <array>
#include <string>
#include <vector>

class GameWorldView;
class PostBox;

struct DeveloperHudResourceChip
{
    std::string icon;
    std::string label;
    std::string value;
};

enum class DeveloperHudFieldAvailability
{
    LiveReadOnly,
    LiveReady,
    Mock,
    Placeholder,
    NotSafelyAccessible
};

struct DeveloperHudDataField
{
    std::string key;
    std::string label;
    std::string value;
    DeveloperHudFieldAvailability availability = DeveloperHudFieldAvailability::Placeholder;
    std::string source;
};

struct DeveloperHudDataGroup
{
    std::string key;
    std::string label;
    std::vector<DeveloperHudDataField> fields;
};

struct DeveloperHudSnapshot
{
    bool isLiveData = false;
    std::string sourceLabel;
    std::string playerName;
    unsigned playerId = 0;
    std::array<unsigned, NUM_SOLDIER_RANKS> soldiersByRank = {};
    unsigned armoredSoldiers = 0;
    unsigned gold = 0;
    unsigned coins = 0;
    unsigned swords = 0;
    unsigned food = 0;
    unsigned boards = 0;
    unsigned stones = 0;
    unsigned messageCount = 0;
    MapExtent mapSize = MapExtent(0, 0);
    MapPoint selectedPoint = MapPoint::Invalid();
    std::string selectedObjectLabel;
};

struct DeveloperHudViewModel
{
    bool isLiveData = false;
    std::string sourceLabel;
    std::string playerLabel;
    std::array<std::string, 5> topBarChips = {};
    std::array<DeveloperHudResourceChip, 6> resourceChips = {};
    std::string militarySummary;
    std::string messageLane;
    std::string selectedSummary;
    std::string mapSummary;
    std::string developmentExportSummary;
    std::vector<DeveloperHudDataGroup> developmentExportGroups;
    unsigned totalSoldiers = 0;
    unsigned messageCount = 0;
};

std::string ToString(DeveloperHudFieldAvailability availability);
DeveloperHudViewModel MakeDeveloperHudViewModel(const DeveloperHudSnapshot& snapshot);

class DeveloperHudDataProvider
{
public:
    virtual ~DeveloperHudDataProvider() = default;
    virtual DeveloperHudViewModel GetViewModel() const = 0;
};

class MockDeveloperHudDataProvider : public DeveloperHudDataProvider
{
public:
    DeveloperHudViewModel GetViewModel() const override;
};

class LiveDeveloperHudDataProvider : public DeveloperHudDataProvider
{
public:
    LiveDeveloperHudDataProvider(const GameWorldView& view, const PostBox* postBox);

    DeveloperHudViewModel GetViewModel() const override;

private:
    const GameWorldView& view_;
    const PostBox* postBox_;
};
