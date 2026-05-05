// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TextureFilter.h"
#include "RTTR_Assert.h"

GLint GetGLTextureFilter(TextureFiltering filtering)
{
    switch(filtering)
    {
        case TextureFiltering::Pixel: return GL_NEAREST;
        case TextureFiltering::Smooth: return GL_LINEAR;
    }
    RTTR_Assert(false);
    return GL_NEAREST;
}

void ApplyTextureFilter(GLint filter)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void ApplyTextureFilter(TextureFiltering filtering)
{
    ApplyTextureFilter(GetGLTextureFilter(filtering));
}
