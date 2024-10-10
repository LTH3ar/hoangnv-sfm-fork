#include "SocialForce.h"

using namespace std;

SocialForce::~SocialForce()
{
    removeCrowd();
    removeWalls();
    removeAGVs();
}

void SocialForce::addAgent(Agent *agent) { crowd.push_back(agent); }

void SocialForce::addWall(Wall *wall) { walls.push_back(wall); }

void SocialForce::addAGV(AGV *agv) { agvs.push_back(agv); }

void SocialForce::removeAgent(int agentId)
{
    if (!crowd.empty())
    {
        crowd.erase(remove_if(begin(crowd), end(crowd), [agentId](Agent *u)
                              { return u->getId() == agentId; }),
                    end(crowd));
    }
}

void SocialForce::removeCrowd()
{
    for (unsigned int idx = 0; idx < crowd.size(); idx++)
        delete crowd[idx];

    crowd.clear();
}

void SocialForce::removeWalls()
{
    for (unsigned int idx = 0; idx < walls.size(); idx++)
        delete walls[idx];

    walls.clear();
}

void SocialForce::removeAGV()
{
    int lastIdx;
    if (!agvs.empty())
    {
        lastIdx = agvs.size() - 1; // Assign index of last element

        delete agvs[lastIdx];
        agvs.pop_back();
    }
}

void SocialForce::removeAGVs()
{
    for (unsigned int idx = 0; idx < crowd.size(); idx++)
        delete agvs[idx];

    agvs.clear();
}

void SocialForce::moveCrowd(float stepTime)
{
    for (unsigned int idx = 0; idx < crowd.size(); idx++)
    {
        // if (crowd[idx]->getIsMoving())
        // {
        //     crowd[idx]->move(crowd, walls, agvs, stepTime);
        // }
        crowd[idx]->move(crowd, walls, agvs, stepTime);
    }
}

void SocialForce::moveAGVs(float stepTime)
{
    vector<Point3f> position_list;
    vector<agvs_data> position_list_agv;
    for (Agent *agent : crowd)
    {
        if (agent->getPosition().distance(Vector3f(0, 0, 0)) > 12.0F)
            continue;
        position_list.push_back(agent->getPosition());
    }

    // add agv position to the position list
    for (AGV *agv : agvs) //assign the position of the AGV with the order of the AGV id
    {
        agvs_data agv_data;
        agv_data.position = agv->getPosition();
        agv_data.id = agv->getId();
        agv_data.reachDestination = agv->getReachDestination();
        position_list_agv.push_back(agv_data);
    }

    for (AGV *agv : agvs)
    {
        if (agv->getIsMoving())
        {
            agv->move(stepTime, position_list, position_list_agv);
        }
    }
}