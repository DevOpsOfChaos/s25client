// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

enum class RulesProfile
{
    RttrCompatible,
    Chaos
};

constexpr RulesProfile GetDefaultRulesProfile()
{
    return RulesProfile::Chaos;
}

constexpr bool IsChaosRulesProfile(const RulesProfile rulesProfile)
{
    return rulesProfile == RulesProfile::Chaos;
}

constexpr const char* GetRulesProfileName(const RulesProfile rulesProfile)
{
    switch(rulesProfile)
    {
        case RulesProfile::RttrCompatible: return "RTTR-compatible";
        case RulesProfile::Chaos: return "Chaos";
    }
    return "Unknown";
}
