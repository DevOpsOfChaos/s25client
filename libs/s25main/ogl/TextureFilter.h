// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "TextureFiltering.h"
#include <glad/glad.h>

GLint GetGLTextureFilter(TextureFiltering filtering);
void ApplyTextureFilter(GLint filter);
void ApplyTextureFilter(TextureFiltering filtering);
