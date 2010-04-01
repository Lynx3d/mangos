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

#include "MapTree.h"
#include "VMapManager2.h"
#include "VMapDefinitions.h"

#include <string>
#include <sstream>
#include <iomanip>

using G3D::Vector3;

namespace VMAP
{
    /* class MapRayCallback {
        public:
            G3D::Vector3 hitLocation;
            G3D::Vector3 hitNormal;
        void operator()(const G3D::Ray& ray, const ModelInstance *entity, bool pStopAtFirstHit, float& distance)
        {
            entity->intersect(ray, distance, pStopAtFirstHit);
            //std::cout << "trying to intersect '" << entity->name << "'\n";
        }
    }; */
    class MapRayCallback
    {
        public:
            MapRayCallback(ModelInstance *val): prims(val) {}
            ModelInstance *prims;
            void operator()(const G3D::Ray& ray, uint32 entry, float& distance, bool pStopAtFirstHit=true)
            {
                prims[entry].intersect(ray, distance, pStopAtFirstHit);
                //std::cout << "trying to intersect '" << entity->name << "'\n";
            }
    };

    class MapPointCallback {
        public:
            MapPointCallback(ModelInstance *val): prims(val) {}
            ModelInstance *prims;
            void operator()(const Vector3& point, uint32 entry)
            {
                //prims[entry].intersect(point, distance, pStopAtFirstHit);
                std::cout << "trying to intersect '" << prims[entry].name << "'\n";
            }
    };

    //=========================================================

    std::string StaticMapTree::getTileFileName(uint32 mapID, uint32 tileX, uint32 tileY)
    {
        std::stringstream tilefilename;
        tilefilename.fill('0');
        tilefilename << std::setw(3) << mapID << "_";
        //tilefilename << std::setw(2) << tileX << "_" << std::setw(2) << tileY << ".vmtile";
        tilefilename << std::setw(2) << tileY << "_" << std::setw(2) << tileX << ".vmtile";
        return tilefilename.str();
    }

    bool StaticMapTree::getAreaInfo(Vector3 pos, unsigned int &areaID, unsigned int &flags)
    {
        std::cout << "StaticMapTree::getAreaInfo()\n";
        MapPointCallback intersectionCallBack(iTreeValues);
        iTree.intersectPoint(pos, intersectionCallBack);

        /* AABox pbox(pos, pos);
        Array<ModelContainer *> mcArray;
        Array<GroupModelBound*> bounds;
        iTree->getIntersectingMembers(pbox, mcArray);
        printf("MapTree::getAreaInfo(): Found %d ModelContainers for position!\n", mcArray.size());
        for(unsigned int i=0; i<mcArray.size(); ++i)
        {
            mcArray[i]->getIntersectingMembers(pos, bounds);
        }
        if(bounds.size()>0)
        {
            unsigned int pick=0;
            float minVolume;
            printf("MapTree::getAreaInfo(): Found %d Bounds for position!\n", bounds.size());
            for(unsigned int i=0; i<bounds.size(); ++i)
            {
                GroupModelBound b=*bounds[i];
                if(!i) minVolume = b.iBound.volume();
                else if (minVolume > b.iBound.volume()){ minVolume=b.iBound.volume(); pick=i; }
                printf("%10u %8X %7.3f,%7.3f,%7.3f | %7.3f,%7.3f,%7.3f | v=%f\n", b.iAreaId, b.iMogpFlags,
                        b.iBound.low().x, b.iBound.low().y, b.iBound.low().z,
                        b.iBound.high().x, b.iBound.high().y, b.iBound.high().z, b.iBound.volume());
            }
            areaID = bounds[pick]->iAreaId;
            flags = bounds[pick]->iMogpFlags;
            return true;
        }
        printf("MapTree::getAreaInfo(): Found no Bounds for position!\n", bounds.size()); */
        return false;
    }

    StaticMapTree::StaticMapTree(uint32 mapID, const std::string &basePath):
        iMapID(mapID), /* iTree(0), */ iTreeValues(0), iBasePath(basePath)
    {
        if(iBasePath.length() > 0 && (iBasePath[iBasePath.length()-1] != '/' || iBasePath[iBasePath.length()-1] != '\\'))
        {
            iBasePath.append("/");
        }
    }

    //=========================================================
    StaticMapTree::~StaticMapTree()
    {
        // TODO
        /* Array<ModelContainer *> mcArray;
        iTree->getMembers(mcArray);
        int no = mcArray.size();
        while(no >0)
        {
            --no;
            delete mcArray[no];
        }
        delete iTree; */
    }
    //=========================================================

    // just for visual debugging with an external debug class
    #ifdef _DEBUG_VMAPS
    #ifndef gBoxArray
    extern Vector3 p1,p2,p3,p4,p5,p6,p7;
    extern Array<AABox>gBoxArray;
    extern int gCount1, gCount2, gCount3, gCount4;
    extern bool myfound;
    #endif
    #endif

    //=========================================================
    /**
    return dist to hit or inf() if no hit
    */

    float StaticMapTree::getIntersectionTime(const G3D::Ray& pRay, float pMaxDist, bool pStopAtFirstHit)
    {
        float distance = pMaxDist;
        MapRayCallback intersectionCallBack(iTreeValues);
        //NodeValueAccess<TreeNode, ModelInstance> vna(iTree, iTreeValues);
        iTree.intersect(pRay, intersectionCallBack, distance/* , pStopAtFirstHit */);
        return distance;
    }
    //=========================================================

    bool StaticMapTree::isInLineOfSight(const Vector3& pos1, const Vector3& pos2)
    {
        bool result = true;
        float maxDist = (pos2 - pos1).magnitude();
        // direction with length of 1
        G3D::Ray ray = G3D::Ray::fromOriginAndDirection(pos1, (pos2 - pos1)/maxDist);
        float resultDist = getIntersectionTime(ray, maxDist, true);
        if(resultDist < maxDist)
        {
            result = false;
        }
        return result;
    }
    //=========================================================
    /**
    When moving from pos1 to pos2 check if we hit an object. Return true and the position if we hit one
    Return the hit pos or the original dest pos
    */

    bool StaticMapTree::getObjectHitPos(const Vector3& pPos1, const Vector3& pPos2, Vector3& pResultHitPos, float pModifyDist)
    {
        bool result=false;
        float maxDist = (pPos2 - pPos1).magnitude();
        Vector3 dir = (pPos2 - pPos1)/maxDist;              // direction with length of 1
        G3D::Ray ray(pPos1, dir);
        float dist = getIntersectionTime(ray, maxDist, false);
        if(dist < maxDist)
        {
            pResultHitPos = pPos1 + dir * dist;
            if(pModifyDist < 0)
            {
                if ((pResultHitPos - pPos1).magnitude() > -pModifyDist)
                {
                    pResultHitPos = pResultHitPos + dir*pModifyDist;
                }
                else
                {
                    pResultHitPos = pPos1;
                }
            }
            else
            {
                pResultHitPos = pResultHitPos + dir*pModifyDist;
            }
            result = true;
        }
        else
        {
            pResultHitPos = pPos2;
            result = false;
        }
        return result;
    }

    //=========================================================

    float StaticMapTree::getHeight(const Vector3& pPos)
    {
        float height = G3D::inf();
        Vector3 dir = Vector3(0,0,-1);
        G3D::Ray ray(pPos, dir);   // direction with length of 1
        float maxDist = VMapDefinitions::getMaxCanFallDistance();
        float dist = getIntersectionTime(ray, maxDist, false);
        if(dist < maxDist)
        {
            height = pPos.z - dist;
        }
        return(height);
    }

    //=========================================================

    bool StaticMapTree::init(const std::string &fname, VMapManager2 *vm)
    {
        std::cout << "Initializing StaticMapTree '" << fname << "'\n";
        bool success=true;
        std::string fullname = iBasePath + fname;
        FILE *rf = fopen(fullname.c_str(), "rb");
        if(!rf)
            return false;
        else
        {
            char chunk[8];
            //general info
            char tiled; // TODO use actual info!
            if (fread(&tiled, sizeof(char), 1, rf) != 1) success = false;
            iIsTiled = (bool(tiled));
            // Nodes
            if (success && fread(chunk, 4, 1, rf) != 1) success = false;
            /* uint32 size;
            if (success && fread(&size, 4, 1, rf) != 1) success = false;
            if (success && fread(&iNNodes, sizeof(uint32), 1, rf) != 1) success = false;
            if (success) iTree = new TreeNode[iNNodes];
            if (success && fread(iTree, sizeof(TreeNode), iNNodes, rf) != iNNodes) success = false;
            // amount of tree elements
            if (success && fread(&iNTreeValues, sizeof(uint32), 1, rf) != 1) success = false;
            if (success) iTreeValues = new ModelInstance[iNTreeValues];
            // __debug__
            if (success) std::cout<< "Allocated " << iNTreeValues << " node values.\n";
            else std::cout << "error reading node value count!\n"; */
            if (success) success = iTree.readFromFile(rf);
            if (success)
            {
                iNTreeValues = iTree.primCount();
                iTreeValues = new ModelInstance[iNTreeValues];
            }

            if (success && fread(chunk, 4, 1, rf) != 1) success = false;
            chunk[4] = 0;
            std::cout << "Chunk: " << &chunk[0] << std::endl;
            // global model spawns
            // only non-tiled maps have them, and if so exactly one (so far at least...)
            ModelSpawn spawn;
            std::cout << "Map isTiled:" << bool(iIsTiled) << std::endl;
            if (!iIsTiled && ModelSpawn::readFromFile(rf, spawn))
            {
                WorldModel *model = vm->aquireModelInstance(iBasePath, spawn.name);
                std::cout << "StaticMapTree::init(): loading " << spawn.name << std::endl;
                if (model)
                {
                    // assume that global model always is the first and only tree value (could be improved...)
                    iTreeValues[0] = ModelInstance(spawn, model);
                    iLoadedSpawns[spawn.ID] = 1;
                }
                else
                {
                    success = false;
                    std::cout << "error: could not acquire WorldModel pointer!\n";
                }
            }

            fclose(rf);
        }
        return success;
    }
    //=========================================================

    /* bool StaticMapTree::PrepareTree()
    {
        iTree->balance();
        return true;
    } */

    bool StaticMapTree::loadMap(uint32 tileX, uint32 tileY, VMapManager2 *vm)
    {
        if (!iIsTiled)
            return true;
        // __debug__
        if(/* !iTree || */ !iTreeValues) std::cout << "Tree has not been initialized!\n";
        bool result = true;
        // if (isTiled)...
        std::string tilefile = iBasePath + getTileFileName(iMapID, tileX, tileY);
        FILE* tf = fopen(tilefile.c_str(), "rb");
        if(tf)
        {
            while(result)
            {
                // read model spawns
                ModelSpawn spawn;
                result = ModelSpawn::readFromFile(tf, spawn);
                if(result)
                {
//                    std::cout << "loading '" << spawn.name << "'\n";

                    // acquire model instance
                    WorldModel *model = vm->aquireModelInstance(iBasePath, spawn.name);
                    if(!model) std::cout << "error: could not acquire WorldModel pointer!\n";

                    // update tree
                    uint32 nNodeVal=0, referencedVal;
                    fread(&nNodeVal, sizeof(uint32), 1, tf);
                    // __debug__
                    if(nNodeVal != 1)
                        std::cout << "unexpected amount of affected NodeVals! (" << nNodeVal << ")\n";
                    // __debug__ end
                    for(uint32 i=0; i<nNodeVal; ++i)
                    {
                        fread(&referencedVal, sizeof(uint32), 1, tf);
                        if(!iLoadedSpawns.count(spawn.ID))
                        {
                            // __debug__
                            if(referencedVal > iNTreeValues)
                            {
                                std::cout << "invalid tree element! (" << referencedVal << "/" << iNTreeValues << ")\n";
                                continue;
                            }
                            // __debug__ end
                            iTreeValues[referencedVal] = ModelInstance(spawn, model);
                            iLoadedSpawns[spawn.ID] = 1;
                        }
                        else
                        {
                            ++iLoadedSpawns[spawn.ID];
                            // __debug__
                            if (iTreeValues[referencedVal].ID != spawn.ID) std::cout << "error: trying to load wrong spawn in node!\n";
                            else if (iTreeValues[referencedVal].name != spawn.name) std::cout << "error: name collision on GUID="<< spawn.ID << "\n";
                            // __debug__ end
                        }
                    }
                }
            }
            iLoadedTiles[packTileID(tileX, tileY)] = true;
            fclose(tf);
        }
        else
            iLoadedTiles[packTileID(tileX, tileY)] = false;
        return result;
    }

    //=========================================================

    void StaticMapTree::unloadMap(uint32 tileX, uint32 tileY, VMapManager2 *vm)
    {
        if (!iIsTiled)
            return;
        uint32 tileID = packTileID(tileX, tileY);
        loadedTileMap::iterator tile = iLoadedTiles.find(tileID);
        if(tile == iLoadedTiles.end())
        {
            std::cout << "WARNING: trying to unload non-loaded tile. Map:" << iMapID << " X:" << tileX << " Y:" << tileY << std::endl;
            return;
        }
        if(tile->second) // file associated with tile
        {
            std::string tilefile = iBasePath + getTileFileName(iMapID, tileX, tileY);
            FILE* tf = fopen(tilefile.c_str(), "rb");
            if(tf)
            {
                bool result=true;
                while(result)
                {
                    // read model spawns
                    ModelSpawn spawn;
                    result = ModelSpawn::readFromFile(tf, spawn);
                    if(result)
                    {
//                        std::cout << "unloading '" << spawn.name << "'\n";

                        // release model instance
                        vm->releaseModelInstance(spawn.name);

                        // update tree
                        uint32 nNodeVal=0, referencedNode;
                        fread(&nNodeVal, sizeof(uint32), 1, tf);
                        for(uint32 i=0; i<nNodeVal; ++i)
                        {
                            fread(&referencedNode, sizeof(uint32), 1, tf);
                            if(!iLoadedSpawns.count(spawn.ID))
                            {
                                std::cout << "error! trying to unload non-referenced model '" << spawn.name << "' (ID:" << spawn.ID << ")\n";
                            }
                            else if(--iLoadedSpawns[spawn.ID] == 0)
                            {
                                std::cout << "MapTree: removing '" << spawn.name << "' from tree\n";
                                iTreeValues[referencedNode].setUnloaded();
                                iLoadedSpawns.erase(spawn.ID);
                            }
                        }
                    }
                }
                fclose(tf);
            }
        }
        iLoadedTiles.erase(tile);
    }

    //=========================================================
    //=========================================================

    /* void StaticMapTree::addModelContainer(const std::string& pName, ManagedModelContainer *pMc)
    {
        iLoadedModelContainer.set(pName, pMc);
        iTree->insert(pMc);
    } */
    //=========================================================
    //=========================================================
    //=========================================================
}