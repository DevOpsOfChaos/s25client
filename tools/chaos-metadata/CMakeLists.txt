# Copyright (C) 2005 - 2026 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(chaos-metadata main.cpp)
target_link_libraries(chaos-metadata PRIVATE s25Main Boost::program_options Boost::nowide)
set_target_properties(chaos-metadata PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(chaos-metadata PRIVATE cxx_std_17)
