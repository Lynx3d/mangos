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

#include "Creature.h"
#include "CreatureAI.h"
#include "MapManager.h"
#include "TrajectoryMovementGenerator.h"

template<>
void
TrajectoryMovementGenerator<Creature>::Initialize(Creature &owner)
{
    owner.GetPosition(i_x0, i_y0, i_z0);
    // do some math; note that time==0 is at destination not start, time is counted down,
    // so all calculation are relative to destination coords.
    float fTime = (float)i_totalFlightTime.GetExpiry()/IN_MILISECONDS;
    i_xspeed = (i_x0 - i_x1) / fTime;
    i_yspeed = (i_y0 - i_y1) / fTime;
    i_zspeed1 = 0.5f * i_gravity * fTime + (i_z0 - i_z1) / fTime;
    owner.addUnitState(UNIT_STAT_FLEEING|UNIT_STAT_FLEEING_MOVE);
sLog.outDetail("TrajectoryMovementGenerator<Creature>::Initialize(): knockbackTime: %u, gravity:%f", i_totalFlightTime.GetExpiry(), i_gravity);
sLog.outDetail("\tfinal speed: %f, p0: %f, %f, %f  p1: %f, %f, %f", i_zspeed1, i_x0, i_y0, i_z0, i_x1, i_y1, i_z1);
    owner.SendMonsterMove(i_x1, i_y1, i_z1, SPLINETYPE_NORMAL, SplineFlags(SPLINEFLAG_WALKMODE|SPLINEFLAG_TRAJECTORY|SPLINEFLAG_KNOCKBACK),
                          i_totalFlightTime.GetExpiry(), NULL, 2.f * i_gravity);
}

template<class T>
void TrajectoryMovementGenerator<T>::Interrupt(T &owner)
{
    // you cannot really interrupt knockback movement...except *maybe* by root, but i doubt

    // flee state still applied while movegen disabled
    owner.clearUnitState(UNIT_STAT_FLEEING_MOVE);
}

template<class T>
void TrajectoryMovementGenerator<T>::Reset(T &owner)
{
    Initialize(owner);
}

template<>
void TrajectoryMovementGenerator<Creature>::Finalize(Creature &owner)
{
    owner.clearUnitState(UNIT_STAT_FLEEING|UNIT_STAT_FLEEING_MOVE);
    if (Unit* victim = owner.getVictim())
    {
        if (owner.isAlive())
        {
            owner.AttackStop(true);
            ((Creature*)&owner)->AI()->AttackStart(victim);
        }
    }
}

template<class T>
bool TrajectoryMovementGenerator<T>::Update(T & owner, const uint32 & time_diff)
{
    // do mobs continue to fly when dead?
    if( !owner.isAlive() )
        return false;

    i_totalFlightTime.Update(time_diff);
    if (i_totalFlightTime.Passed())
    {
        owner.GetMap()->CreatureRelocation(&owner, i_x1, i_y1, i_z1, owner.GetOrientation());
        return false;
    }

    float fTime = (float)i_totalFlightTime.GetExpiry()/IN_MILISECONDS;
    float curr_z = i_z1 + i_zspeed1 * fTime - 0.5f * i_gravity * fTime * fTime;
sLog.outDetail("Trajectory Update: Pos: %f, %f, %f", i_x1 + fTime*i_xspeed, i_y1 + fTime*i_yspeed, curr_z);
    owner.GetMap()->CreatureRelocation(&owner, i_x1 + fTime*i_xspeed, i_y1 + fTime*i_yspeed, curr_z, owner.GetOrientation());

    return true;
}

template bool TrajectoryMovementGenerator<Creature>::Update(Creature & owner, const uint32 & time_diff);
template void TrajectoryMovementGenerator<Creature>::Interrupt(Creature &owner);
template void TrajectoryMovementGenerator<Creature>::Reset(Creature &owner);
