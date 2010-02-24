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

#ifndef MANGOS_TRAJECTORYMOVEMENTGENERATOR_H
#define MANGOS_TRAJECTORYMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class MANGOS_DLL_SPEC TrajectoryMovementGenerator
: public MovementGeneratorMedium< Creature, TrajectoryMovementGenerator >
{
    public:
        TrajectoryMovementGenerator(float x, float y, float z, float gravity, uint32 duration) :
            i_x1(x), i_y1(y), i_z1(z), i_gravity(gravity), i_totalFlightTime(duration)
        {
        }

        void Initialize(Creature &);
        void Finalize(Creature &);
        void Interrupt(Creature &);
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &);

        MovementGeneratorType GetMovementGeneratorType() { return TRAJECTORY_MOTION_TYPE; }
    private:
        float i_x0;
        float i_y0;
        float i_z0;
        float i_x1;
        float i_y1;
        float i_z1;
        float i_xspeed;
        float i_yspeed;
        float i_zspeed1; //!< final fall speed
        float i_gravity;
        TimeTracker i_totalFlightTime;
};

#endif
