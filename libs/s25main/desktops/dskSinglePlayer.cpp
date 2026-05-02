// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dskSinglePlayer.h"
#include "ChaosCompatibilityPreview.h"
#include "ListDir.h"
#include "Loader.h"
#include "RttrConfig.h"
#include "Savegame.h"
#include "Settings.h"
#include "WindowManager.h"
#include "commonDefines.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlText.h"
#include "dskCampaignSelection.h"
#include "dskMainMenu.h"
#include "dskSelectMap.h"
#include "files.h"
#include "ingameWindows/iwConnecting.h"
#include "ingameWindows/iwMsgbox.h"
#include "ingameWindows/iwPlayReplay.h"
#include "ingameWindows/iwSave.h"
#include "network/CreateServerInfo.h"
#include "network/GameClient.h"

namespace bfs = boost::filesystem;

static CreateServerInfo createLocalGameInfo(const std::string& name)
{
    return CreateServerInfo(ServerType::Local, SETTINGS.server.localPort, name);
}

namespace {
enum
{
    ID_txtResumeCompatibility = 12
};

unsigned GetCompatibilityPreviewColor(const chaos::CompatibilityPreviewStatus status)
{
    switch(status)
    {
        case chaos::CompatibilityPreviewStatus::Neutral: return COLOR_YELLOW;
        case chaos::CompatibilityPreviewStatus::Compatible: return COLOR_GREEN;
        case chaos::CompatibilityPreviewStatus::Invalid:
        case chaos::CompatibilityPreviewStatus::Incompatible: return COLOR_RED;
    }
    return COLOR_RED;
}

bfs::path FindMostRecentSavePath()
{
    const std::vector<bfs::path> savFiles = ListDir(RTTRCONFIG.ExpandPath(s25::folders::save), "sav");

    bfs::path mostRecentFilepath;
    s25util::time64_t recent = 0;
    for(const auto& savFile : savFiles)
    {
        Savegame save;

        if(!save.Load(savFile, SaveGameDataToLoad::Header))
            continue;

        if(save.GetSaveTime() > recent)
        {
            recent = save.GetSaveTime();
            mostRecentFilepath = savFile;
        }
    }

    return mostRecentFilepath;
}
} // namespace

/** @class dskSinglePlayer
 *
 *  Klasse des Einzelspieler Desktops.
 */

dskSinglePlayer::dskSinglePlayer()
{
    RTTR_Assert(dskMenuBase::ID_FIRST_FREE <= 3);

    AddTextButton(3, DrawPoint(115, 180), Extent(220, 22), TextureColor::Green2, _("Resume last game"), NormalFont);
    AddTextButton(7, DrawPoint(115, 210), Extent(220, 22), TextureColor::Green2, _("Load game"), NormalFont);

    AddTextButton(5, DrawPoint(115, 250), Extent(220, 22), TextureColor::Green2, std::string(_("Campaigns")),
                  NormalFont);
    AddTextButton(6, DrawPoint(115, 280), Extent(220, 22), TextureColor::Green2, _("Unlimited Play"), NormalFont);

    AddTextButton(4, DrawPoint(115, 320), Extent(220, 22), TextureColor::Green2, _("Play Replay"), NormalFont);

    AddTextButton(8, DrawPoint(115, 390), Extent(220, 22), TextureColor::Red1, _("Back"), NormalFont);

    AddImage(11, DrawPoint(20, 20), LOADER.GetImageN("logo", 0));

    if(const bfs::path mostRecentFilepath = FindMostRecentSavePath(); !mostRecentFilepath.empty())
    {
        const chaos::CompatibilityPreview compatibilityPreview = chaos::BuildCompatibilityPreview(
          chaos::EvaluateContentCompatibility(mostRecentFilepath, SETTINGS.chaos.rulesProfile));
        AddText(ID_txtResumeCompatibility, DrawPoint(345, 184), compatibilityPreview.text,
                GetCompatibilityPreviewColor(compatibilityPreview.status), FontStyle::LEFT, SmallFont)
          ->setMaxWidth(420);
    }
}

void dskSinglePlayer::Msg_ButtonClick(const unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case 3: // "Letztes Spiel fortsetzen"
        {
            const bfs::path mostRecentFilepath = FindMostRecentSavePath();

            if(!mostRecentFilepath.empty())
            {
                // Dateiname noch rausextrahieren aus dem Pfad
                if(!mostRecentFilepath.has_filename())
                    return;
                const auto name = mostRecentFilepath.stem().string();

                // Server info
                CreateServerInfo csi = createLocalGameInfo(name);

                WINDOWMANAGER.Switch(std::make_unique<dskSelectMap>(csi));

                if(GAMECLIENT.HostGame(csi, {mostRecentFilepath, MapType::Savegame}))
                    WINDOWMANAGER.ShowAfterSwitch(std::make_unique<iwConnecting>(csi.type, nullptr));
                else
                {
                    const std::string error = GAMECLIENT.GetLastHostError().empty() ?
                                                _("The specified file couldn't be loaded!") :
                                                GAMECLIENT.GetLastHostError();
                    WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Error"), error, nullptr, MsgboxButton::Ok,
                                                                  MsgboxIcon::ExclamationRed));
                }
            } else
                WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Error"), _("The specified file couldn't be loaded!"),
                                                              nullptr, MsgboxButton::Ok, MsgboxIcon::ExclamationRed));
        }
        break;
        case 4: // "Replay abspielen"
        {
            WINDOWMANAGER.ToggleWindow(std::make_unique<iwPlayReplay>());
        }
        break;
        case 5: // "Kampagne"
        {
            WINDOWMANAGER.Switch(std::make_unique<dskCampaignSelection>(createLocalGameInfo(_("Campaign"))));
        }
        break;
        case 6: // "Freies Spiel"
        {
            PrepareSinglePlayerServer();
        }
        break;
        case 7: // "Spiel laden"
        {
            PrepareLoadGame();
        }
        break;
        case 8: // "Zurück"
        {
            WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());
        }
        break;
    }
}

void dskSinglePlayer::PrepareSinglePlayerServer()
{
    WINDOWMANAGER.Switch(std::make_unique<dskSelectMap>(createLocalGameInfo(_("Unlimited Play"))));
}

void dskSinglePlayer::PrepareLoadGame()
{
    CreateServerInfo csi = createLocalGameInfo(_("Unlimited Play"));

    WINDOWMANAGER.Switch(std::make_unique<dskSelectMap>(csi));
    WINDOWMANAGER.ShowAfterSwitch(std::make_unique<iwLoad>(csi));
}
