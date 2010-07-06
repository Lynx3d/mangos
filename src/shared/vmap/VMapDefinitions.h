/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _VMAPDEFINITIONS_H
#define _VMAPDEFINITIONS_H
#include <cstring>

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
    //=====================================
    #define MAX_CAN_FALL_DISTANCE 10.0f
    const char VMAP_MAGIC[] = "VMAP_3.0";

    class VMapDefinitions
    {
        public:
            static float getMaxCanFallDistance() { return MAX_CAN_FALL_DISTANCE; }
    };

    //======================================

    // defined in TileAssembler.cpp currently...
    bool readChunk(FILE *rf, char *dest, const char *compare, uint32 len);
}

#ifndef NO_CORE_FUNCS
    #include "Errors.h"
    #include "Log.h"
    #define ERROR_LOG(...) sLog.outDebug(__VA_ARGS__);
#else
    #define ASSERT(x) do{} while(0)
    #define DEBUG_LOG(...) do{} while(0)
    #define ERROR_LOG(...) do{} while(0)
    #define DETAIL_LOG(...) do{} while(0)
#endif

#endif // _VMAPDEFINITIONS_H
