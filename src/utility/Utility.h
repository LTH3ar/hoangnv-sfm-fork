#ifndef UTILITES_H
#define UTILITES_H

#include <vector>
#include <iostream>
#include <array>
#include <lib/nlohmann/json.hpp>
#include "lib/vecmath/vecmath.h"
#include <map>
#include "src/agv/AGV.h"
#include "src/agent/Agent.h"

using json = nlohmann::json;

#pragma once
namespace Utility
{
    float randomFloat(float lowerBound, float upperBound);

    std::map<std::string, std::vector<float>> readMapData(const char *fileName);

    std::vector<json> convertMapData(std::map<std::string, std::vector<float>> mapData);

    bool hallwaySameCharExists(float hallwayLength, std::vector<json> data);

    json readInputData(const char *fileName);

    int calculatePredictedTime(float hallwayLength, float desiredSpeed, float acceleration, int timeRatio);

    void writeResult(
        const char *fileName, std::string name, int mode, std::vector<AGV *> data,
        std::vector<json> juncDataList,
        int agvRunConcurrently,
        int runMode,
        int numRunPerHallway,
        int totalRunningTime,
        int timeRatio);

    json SaveState(
        // list of AGVs
        std::vector<AGV *> agvs,
        // list of Agents
        std::vector<Agent *> agents,
        // time stamp (ms)
        int time
    );

    void writeState(
        const char *fileName,
        std::vector<json> stateList
    );

    std::vector<int> getNumPedesInFlow(int junctionType, int totalPedestrian);

    std::vector<double> getPedesVelocity(int type, json inputData, float deviationParam);

    std::vector<double> getPedesVelocityBasedTDis(int numPedes, double n_dist);

    std::vector<double> getPedesVelocityBasedDDis(json inputData, float deviationParam);

    std::vector<float> getWallCoordinates(float walkwayWidth, std::vector<float> juncData);

    std::string convertTime(int milliseconds);

    std::vector<float> getMapLimit(float walkwayWidth, std::vector<float> juncData);

    std::vector<float> getPedesDestination(int direction, int side, float walkwayWidth, std::vector<float> juncData, bool stopAtCorridor);

    std::vector<float> getPedesSource(int direction, float totalLength, float subLength, float caravanWidth, float walkwayWidth, std::vector<float> juncData);

    std::vector<float> getPedesColor(float maxSpeed, float minSpeed, float desiredSpeed, int type);

    std::vector<Point3f> getRouteAGV(int src, int turningDirection, float walkwayWidth, std::vector<float> juncData);

    std::vector<Point3f> getRouteAGVCrossRoad(int src, int turningDirection, float walkwayWidth, std::vector<float> juncData);

    std::vector<Point3f> getRouteAGVTJunction(int src, int turningDirection, float walkwayWidth, std::vector<float> juncData);

    std::vector<Point3f> getRouteAGVHallway(int src, float walkwayWidth, std::vector<float> juncData);

    Point3f getIntermediateDes(Point3f position, float verWalkwayWidth, float horWalkwayWidth);

    bool isPositionErr(Point3f position, float walkwayWidth, int junctionType, std::vector<AGV *> agvs);

    int getNumAGVCompleted(std::vector<AGV *> agvs);

    int randomInt(int from, int to);
};

#endif