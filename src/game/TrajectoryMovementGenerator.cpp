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

void
TrajectoryMovementGenerator::Initialize(Creature &owner)
{
    owner.GetPosition(i_x0, i_y0, i_z0);
    // do some math; note that time==0 is at destination not start, time is counted down,
    // so all calculation are relative to destination coords.
    float fTime = (float)i_totalFlightTime.GetExpiry()/IN_MILISECONDS;
    i_xspeed = (i_x0 - i_x1) / fTime;
    i_yspeed = (i_y0 - i_y1) / fTime;
    i_zspeed1 = 0.5f * i_gravity * fTime + (i_z0 - i_z1) / fTime;
    owner.addUnitState(UNIT_STAT_TRAJECTORY);
sLog.outDetail("TrajectoryMovementGenerator<Creature>::Initialize(): knockbackTime: %u, gravity:%f", i_totalFlightTime.GetExpiry(), i_gravity);
sLog.outDetail("\tfinal speed: %f, p0: %f, %f, %f  p1: %f, %f, %f", i_zspeed1, i_x0, i_y0, i_z0, i_x1, i_y1, i_z1);
    owner.SendMonsterMove(i_x1, i_y1, i_z1, SPLINETYPE_NORMAL, SplineFlags(SPLINEFLAG_WALKMODE|SPLINEFLAG_TRAJECTORY|SPLINEFLAG_KNOCKBACK),
                          i_totalFlightTime.GetExpiry(), NULL, 2.f * i_gravity);
}

void TrajectoryMovementGenerator::Interrupt(Creature &owner)
{
    // TODO: analyze situations where other movements would be applied on top of trajectory
    // only charge and *maybe* root effects should be able to interrupt knockback movement...

    owner.clearUnitState(UNIT_STAT_TRAJECTORY);
}

void TrajectoryMovementGenerator::Reset(Creature &owner)
{
    // TODO: analyze situations where Reset will be called, and how to react
    // trajectory parameters become invalid once movement interrupted. Possibly set timer to zero?

    Initialize(owner);
}

void TrajectoryMovementGenerator::Finalize(Creature &owner)
{
    owner.clearUnitState(UNIT_STAT_TRAJECTORY);
    if (Unit* victim = owner.getVictim())
    {
        if (owner.isAlive())
        {
            owner.AttackStop(true);
            ((Creature*)&owner)->AI()->AttackStart(victim);
        }
    }
}

bool TrajectoryMovementGenerator::Update(Creature &owner, const uint32 &time_diff)
{
    // do mobs continue to fly/fall when dead?
    /* if( !owner.isAlive() )
        return false; */

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
