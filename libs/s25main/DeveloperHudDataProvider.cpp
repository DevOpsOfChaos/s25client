// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DeveloperHudDataProvider.h"
#include "GamePlayer.h"
#include "postSystem/PostBox.h"
#include "world/GameWorldBase.h"
#include "world/GameWorldView.h"
#include "world/GameWorldViewer.h"
#include "nodeObjs/noBase.h"
#include "gameTypes/GoodTypes.h"
#include <numeric>
#include <string>

namespace {
std::string FormatCount(const unsigned value)
{
    return std::to_string(value);
}

std::string FormatPoint(const MapPoint pt)
{
    return std::to_string(pt.x) + "," + std::to_string(pt.y);
}

std::string GetObjectLabel(const GO_Type objectType)
{
    switch(objectType)
    {
        case GO_Type::Nothing: return "empty node";
        case GO_Type::NobHq: return "headquarters";
        case GO_Type::NobMilitary: return "military building";
        case GO_Type::NobStorehouse: return "storehouse";
        case GO_Type::NobUsual: return "building";
        case GO_Type::NobHarborbuilding: return "harbor";
        case GO_Type::Buildingsite: return "building site";
        case GO_Type::Flag: return "flag";
        case GO_Type::Tree: return "tree";
        case GO_Type::Ware: return "ware";
        case GO_Type::Ship: return "ship";
        default: return "object";
    }
}

DeveloperHudFieldAvailability GetSnapshotAvailability(const DeveloperHudSnapshot& snapshot)
{
    return snapshot.isLiveData ? DeveloperHudFieldAvailability::LiveReadOnly : DeveloperHudFieldAvailability::LiveReady;
}

DeveloperHudDataField MakeField(const std::string& key, const std::string& label, const std::string& value,
                                const DeveloperHudFieldAvailability availability, const std::string& source)
{
    return DeveloperHudDataField{key, label, value, availability, source};
}

unsigned CountExportFields(const std::vector<DeveloperHudDataGroup>& groups)
{
    unsigned fieldCount = 0;
    for(const DeveloperHudDataGroup& group : groups)
        fieldCount += static_cast<unsigned>(group.fields.size());
    return fieldCount;
}
} // namespace

std::string ToString(const DeveloperHudFieldAvailability availability)
{
    switch(availability)
    {
        case DeveloperHudFieldAvailability::LiveReadOnly: return "live read-only";
        case DeveloperHudFieldAvailability::LiveReady: return "live-ready field";
        case DeveloperHudFieldAvailability::Mock: return "mock";
        case DeveloperHudFieldAvailability::Placeholder: return "placeholder";
        case DeveloperHudFieldAvailability::NotSafelyAccessible: return "not safely accessible yet";
    }
    return "unknown";
}

DeveloperHudViewModel MakeDeveloperHudViewModel(const DeveloperHudSnapshot& snapshot)
{
    DeveloperHudViewModel viewModel;
    viewModel.isLiveData = snapshot.isLiveData;
    viewModel.sourceLabel = snapshot.sourceLabel;
    viewModel.totalSoldiers = std::accumulate(snapshot.soldiersByRank.begin(), snapshot.soldiersByRank.end(), 0u);
    viewModel.messageCount = snapshot.messageCount;
    viewModel.playerLabel = "P" + std::to_string(snapshot.playerId + 1) + " " + snapshot.playerName;

    viewModel.topBarChips = {viewModel.playerLabel, "S " + FormatCount(viewModel.totalSoldiers),
                             "Au " + FormatCount(snapshot.gold), "Sw " + FormatCount(snapshot.swords),
                             "Msg " + FormatCount(snapshot.messageCount)};

    viewModel.resourceChips = {{{"Au", "Gold", FormatCount(snapshot.gold)},
                                {"Sw", "Swords", FormatCount(snapshot.swords)},
                                {"Fd", "Food", FormatCount(snapshot.food)},
                                {"Co", "Coins", FormatCount(snapshot.coins)},
                                {"Bd", "Boards", FormatCount(snapshot.boards)},
                                {"St", "Stones", FormatCount(snapshot.stones)}}};

    viewModel.militarySummary =
      "Soldiers " + FormatCount(viewModel.totalSoldiers) + " | Ranks " + FormatCount(snapshot.soldiersByRank[0]) + "/"
      + FormatCount(snapshot.soldiersByRank[1]) + "/" + FormatCount(snapshot.soldiersByRank[2]) + "/"
      + FormatCount(snapshot.soldiersByRank[3]) + "/" + FormatCount(snapshot.soldiersByRank[4]) + " | Armor "
      + FormatCount(snapshot.armoredSoldiers);
    viewModel.messageLane = snapshot.messageCount == 0 ?
                              "Message lane: no unread post messages" :
                              "Message lane: " + FormatCount(snapshot.messageCount) + " unread post messages";
    viewModel.selectedSummary = snapshot.selectedPoint.isValid() ? "Selected " + FormatPoint(snapshot.selectedPoint)
                                                                     + " | " + snapshot.selectedObjectLabel :
                                                                   "Selected: no map point";
    viewModel.mapSummary =
      "Map " + FormatCount(snapshot.mapSize.x) + "x" + FormatCount(snapshot.mapSize.y) + " | read-only";
    const DeveloperHudFieldAvailability primaryAvailability = GetSnapshotAvailability(snapshot);
    const std::string source = snapshot.sourceLabel;
    const std::string totalSoldiers = FormatCount(viewModel.totalSoldiers);
    const std::string soldierRanks =
      FormatCount(snapshot.soldiersByRank[0]) + "/" + FormatCount(snapshot.soldiersByRank[1]) + "/"
      + FormatCount(snapshot.soldiersByRank[2]) + "/" + FormatCount(snapshot.soldiersByRank[3]) + "/"
      + FormatCount(snapshot.soldiersByRank[4]);
    const std::string selectedPoint = snapshot.selectedPoint.isValid() ? FormatPoint(snapshot.selectedPoint) : "none";

    viewModel.developmentExportGroups = {
      {"player",
       "Player / active view",
       {MakeField("player.active.id", "Active player id", FormatCount(snapshot.playerId), primaryAvailability, source),
        MakeField("player.active.name", "Active player name", snapshot.playerName, primaryAvailability, source),
        MakeField("player.active.label", "HUD player label", viewModel.playerLabel, primaryAvailability, source)}},
      {"military",
       "Soldier count / military placeholders",
       {MakeField("military.soldiers.total", "Total soldiers", totalSoldiers, primaryAvailability, source),
        MakeField("military.soldiers.ranks", "Soldier ranks", soldierRanks, primaryAvailability, source),
        MakeField("military.soldiers.armored", "Armored soldiers", FormatCount(snapshot.armoredSoldiers),
                  primaryAvailability, source),
        MakeField("military.status.label", "Military status label", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future military summary accessor"),
        MakeField("military.capacity.used", "Military capacity used", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future military capacity accessor"),
        MakeField("military.capacity.max", "Military capacity max", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future military capacity accessor")}},
      {"resources",
       "Important resources / wares",
       {MakeField("resources.gold", "Gold", FormatCount(snapshot.gold), primaryAvailability, source),
        MakeField("resources.coins", "Coins", FormatCount(snapshot.coins), primaryAvailability, source),
        MakeField("resources.swords", "Swords", FormatCount(snapshot.swords), primaryAvailability, source),
        MakeField("resources.food", "Food", FormatCount(snapshot.food), primaryAvailability, source),
        MakeField("resources.boards", "Boards", FormatCount(snapshot.boards), primaryAvailability, source),
        MakeField("resources.stones", "Stones", FormatCount(snapshot.stones), primaryAvailability, source)}},
      {"messages",
       "Notification lane / post",
       {MakeField("messages.unread.count", "Unread message count", FormatCount(snapshot.messageCount),
                  primaryAvailability, source),
        MakeField("messages.latest.label", "Latest message label", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future post message summary accessor"),
        MakeField("messages.lane.text", "Message lane text", viewModel.messageLane, primaryAvailability, source)}},
      {"selection",
       "Selection / context panel",
       {MakeField("selection.map.point", "Selected map point", selectedPoint, primaryAvailability, source),
        MakeField("selection.object.summary", "Selected object summary", viewModel.selectedSummary, primaryAvailability,
                  source),
        MakeField("selection.context.actions", "Context actions", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future selected object command accessor")}},
      {"map",
       "Map / minimap",
       {MakeField("map.size", "Map size", FormatCount(snapshot.mapSize.x) + "x" + FormatCount(snapshot.mapSize.y),
                  primaryAvailability, source),
        MakeField("minimap.thumbnail", "Minimap thumbnail", "placeholder card",
                  DeveloperHudFieldAvailability::Placeholder, "future minimap renderer export"),
        MakeField("minimap.viewport", "Current viewport", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future GameWorldView viewport accessor")}},
      {"commands",
       "Command rail / quick actions",
       {MakeField("commands.rail.categories", "Command rail categories", "Build | Road | Military | Economy | Post",
                  DeveloperHudFieldAvailability::Placeholder, "developer UI contract"),
        MakeField("commands.quick.build.labels", "Build quick action labels",
                  "House | Road | Flag | Mine | Storehouse | Cancel", DeveloperHudFieldAvailability::Placeholder,
                  "developer UI contract"),
        MakeField("commands.quick.build.eligibility", "Build quick action eligibility", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future read-only action eligibility model"),
        MakeField("commands.dispatch", "Command dispatch", "disabled / no commands",
                  DeveloperHudFieldAvailability::Placeholder, "developer preview safety")}},
      {"economy",
       "Economy / production status",
       {MakeField("economy.storage.pressure", "Storage pressure", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future economy summary accessor"),
        MakeField("economy.production.alerts", "Production alerts", "not exported yet",
                  DeveloperHudFieldAvailability::NotSafelyAccessible, "future production alert accessor")}}};

    viewModel.developmentExportSummary =
      "HUD export contract: " + FormatCount(static_cast<unsigned>(viewModel.developmentExportGroups.size()))
      + " groups / " + FormatCount(CountExportFields(viewModel.developmentExportGroups)) + " fields";
    return viewModel;
}

DeveloperHudViewModel MockDeveloperHudDataProvider::GetViewModel() const
{
    DeveloperHudSnapshot snapshot;
    snapshot.isLiveData = false;
    snapshot.sourceLabel = "Static mock data";
    snapshot.playerName = "Mock player";
    snapshot.playerId = 0;
    snapshot.soldiersByRank = {{42, 91, 36, 12, 3}};
    snapshot.armoredSoldiers = 28;
    snapshot.gold = 74;
    snapshot.coins = 1256;
    snapshot.swords = 38;
    snapshot.food = 214;
    snapshot.boards = 63;
    snapshot.stones = 47;
    snapshot.messageCount = 3;
    snapshot.mapSize = MapExtent(96, 96);
    snapshot.selectedPoint = MapPoint(44, 52);
    snapshot.selectedObjectLabel = "headquarters mock";
    return MakeDeveloperHudViewModel(snapshot);
}

LiveDeveloperHudDataProvider::LiveDeveloperHudDataProvider(const GameWorldView& view, const PostBox* postBox)
    : view_(view), postBox_(postBox)
{}

DeveloperHudViewModel LiveDeveloperHudDataProvider::GetViewModel() const
{
    const GameWorldViewer& viewer = view_.GetViewer();
    const GamePlayer& player = viewer.GetPlayer();
    const Inventory& inventory = player.GetInventory();

    DeveloperHudSnapshot snapshot;
    snapshot.isLiveData = true;
    snapshot.sourceLabel = "Live read-only game data";
    snapshot.playerName = player.name;
    snapshot.playerId = viewer.GetPlayerId();
    for(unsigned rank = 0; rank < NUM_SOLDIER_RANKS; ++rank)
    {
        const Job soldierJob = SOLDIER_JOBS[rank];
        snapshot.soldiersByRank[rank] = inventory[soldierJob];
        snapshot.armoredSoldiers += inventory[jobEnumToAmoredSoldierEnum(soldierJob)];
    }
    snapshot.gold = inventory[GoodType::Gold];
    snapshot.coins = inventory[GoodType::Coins];
    snapshot.swords = inventory[GoodType::Sword];
    snapshot.food = inventory[GoodType::Fish] + inventory[GoodType::Bread] + inventory[GoodType::Meat];
    snapshot.boards = inventory[GoodType::Boards];
    snapshot.stones = inventory[GoodType::Stones];
    snapshot.messageCount = postBox_ ? postBox_->GetNumMsgs() : 0;
    snapshot.mapSize = viewer.GetWorld().GetSize();
    snapshot.selectedPoint = view_.GetSelectedPt();
    if(snapshot.selectedPoint.isValid())
        snapshot.selectedObjectLabel = GetObjectLabel(viewer.GetWorld().GetNO(snapshot.selectedPoint)->GetGOT());
    return MakeDeveloperHudViewModel(snapshot);
}
