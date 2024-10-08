#include "Utility.h"

#include <bits/stdc++.h>

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "src/constant/Constant.h"

using namespace std;
using namespace Constant;
using namespace Utility;

// random float number between particular range
float Utility::randomFloat(float lowerBound, float upperBound)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(lowerBound, upperBound);
    return dis(gen);
}

// calculate predicted time
int Utility::calculatePredictedTime(float hallwayLength, float desiredSpeed, float acceleration, float timeRatio) {
    // Time to reach desired speed
    float t1 = desiredSpeed / acceleration;
    
    // Distance covered during acceleration
    float d1 = 0.5 * acceleration * t1 * t1;
    
    // Check if the distance covered during acceleration is less than the total distance
    if (d1 <= hallwayLength) {
        // Remaining distance after reaching desired speed
        float d2 = hallwayLength - d1;
        
        // Time to cover the remaining distance at constant speed
        float t2 = d2 / desiredSpeed;
        
        // Total time
        return (float)(t1 + t2)*1000.0f;
    } else {
        // If the object does not reach the desired speed, solve for time using quadratic equation
        // d = 0.5 * a * t^2
        // 0.5 * a * t^2 = d
        // t^2 = 2 * d / a
        // t = sqrt(2 * d / a)
        return (float)sqrt(2 * hallwayLength / acceleration)*1000.0f;
    }
}

// read map data file
std::map<std::string, std::vector<float>> Utility::readMapData(
    const char *fileName)
{
    map<std::string, std::vector<float>> map;
    ifstream input(fileName);

    std::string delimiter = " ";

    int lineNo = 1;

    for (std::string line; getline(input, line);)
    {
        // cout << "Line: " << line << endl;
        vector<float> v;
        if (lineNo == 1)
        {
            v.push_back(stof(line));
            map["numI"] = v;
        }
        else if (lineNo == 2)
        {
            v.push_back(stof(line));
            map["walkwayWidth"] = v;
        }
        else
        {
            // cout << "Line: " <<line << endl;
            size_t pos = 0;
            std::string token;
            int count = 0;
            std::string juncId;
            while ((pos = line.find(delimiter)) != std::string::npos)
            {
                token = line.substr(0, pos);
                // std::cout << token << std::endl;
                if (count == 0)
                {
                    juncId.assign(token);
                }

                if (count > 1)
                {
                    v.push_back(stof(token));
                }
                line.erase(0, pos + delimiter.length());
                count++;
            }
            // cout << line << endl;
            v.push_back(stof(line));
            map[juncId] = v;
        }
        lineNo++;
    }

    return map;
}

std::vector<json> Utility::convertMapData(
    std::map<std::string, std::vector<float>> mapData)
{
    std::vector<json> data;
    for (auto elem : mapData)
    {
        if (elem.first == "numI" || elem.first == "walkwayWidth")
        {
            continue;
        }
        int numOfHallway = elem.second.size();
        for (int i = 0; i < numOfHallway; i++)
        {
            if (!hallwaySameCharExists(elem.second[i], data))
            {
                json temp;
                temp[elem.first + "_" + std::to_string(i)] = elem.second[i];
                data.push_back(temp);
            }
        }
    }
    return data;
}

bool Utility::hallwaySameCharExists(float hallwayLength,
                                    std::vector<json> data)
{
    for (auto elem : data)
    {
        if (fabs(hallwayLength - (float)(elem.items().begin().value())) < 0.1)
        {
            return true;
        }
    }
    return false;
}

// read input file
json Utility::readInputData(const char *fileName)
{
    std::ifstream f(fileName);
    json data = json::parse(f);

    return data;
}

void CleanUpData(const char *folderName)
{
    //remove file has "result_agv_" in the name and end with ".json"
    std::string folder(folderName);
    std::string search = "result_agv_";
    std::string ext = ".json";
    for (const auto &entry : std::filesystem::directory_iterator(folder))
    {
        std::string filename = entry.path().filename().string();
        if (filename.find(search) != std::string::npos && filename.find(ext) != std::string::npos)
        {
            std::filesystem::remove(entry.path());
        }
    }

}

// write end file
void Utility::writeResult(const char *fileName, string name, int mode,
                          std::vector<AGV *> agvs,
                          std::vector<json> juncDataList, int runMode,
                          int totalRunningTime, string arcID, int timeRatio, int timeline_pointer, int eventType, vector<int> NewAgvIDs)
{
    CleanUpData("data/output");
    json j;

    int easyReadingMode = 1;
    int jsonOutput = 1;
    std::string delimiter = " - ";
    std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (runMode == 0)
    {
        for (AGV *agv : agvs)
        {
            string array1[] = {"From Left", "From Bottom", "From Right", "From Top"};
            string array2[] = {"Turn Right", "Go straight", "Turn Left"};
            string direction = array1[(int)(agv->getDirection().x)] + "-" +
                               array2[(int)(agv->getDirection().y)];
        }
    }
    else
    {
        // if (jsonOutput == 1)
        // {
        //     j["Completed_on"] = std::ctime(&now);
        // }

        string hallwayName;
        float hallwayLength;
        vector<int> travelingTimeList;

        int juncIndexTemp = 0;
        int agv_counter = 0;
        for (AGV *agv : agvs)
        {
            agv_counter++;
            string json_filename = "data/output/result_agv_" + std::to_string(agv->getId()) + ".json";
            // clean up file
            std::ofstream ofs;
            ofs.open(json_filename, std::ofstream::out | std::ofstream::trunc);
            ofs.close();

            ofstream JsonOutput(json_filename, ios::app);
            if (agv->getNumOfCollision() == 0)
            {
                agv->setTotalStopTime(0);
            }
            int marker = agvs.size();// * (juncIndexTemp + 1);
            
            travelingTimeList.push_back(agv->getTravelingTime());

            if (juncIndexTemp < juncDataList.size())
            {
                hallwayName = juncDataList[juncIndexTemp].items().begin().key();
                hallwayLength = juncDataList[juncIndexTemp].items().begin().value();

                if (jsonOutput == 1)
                {
                    j["hallwayName"] = arcID;
                    j["hallwayLength"] = hallwayLength;
                    j["agvId"] = agv->getId();
                    j["generalDirection"] = agv->getGeneralDirection();
                    j["travelingTime"] = agv->getTravelingTime()*(timeRatio);
                    j["numOfCollision"] = agv->getNumOfCollision();
                    j["totalStopTime"] = agv->getTotalStopTime()*(timeRatio);
                    if ((int)((agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio)) < (int)((hallwayLength / 0.6) * 1000))
                    {
                        cout << "false" << endl;
                        cout << (int)((agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio)) << endl;
                        cout << (int)((hallwayLength / 0.6) * 1000) << endl;
                        j["AGVRealTime"] = (int)((hallwayLength / 0.6) * 1000) + (agv->getTotalStopTime()*(timeRatio));
                        j["AGVCurrTime"] = (int)((hallwayLength / 0.6) * 1000) + (agv->getTotalStopTime()*(timeRatio))+(timeline_pointer*1000);    
                    }
                    else {
                        cout << "true" << endl;
                        j["AGVRealTime"] = (int)(agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio);
                        j["AGVCurrTime"] = (agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio)+(timeline_pointer*1000);
                    }
                    
                    if (eventType == 1)
                    {
                        for (int i = 0; i < NewAgvIDs.size(); i++)
                        {
                            if (agv->getId() == NewAgvIDs[i])
                            {
                                if ((int)((agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio)) < (int)((hallwayLength / 0.6) * 1000))
                                {
                                    cout << "false" << endl;
                                    cout << (int)(agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio) << endl;
                                    cout << (int)((hallwayLength / 0.6) * 1000) << endl;
                                    j["AGVCurrTime"] = (hallwayLength / 0.6) * 1000 + (agv->getTotalStopTime()*(timeRatio));    
                                }
                                else {
                                    cout << "true" << endl;
                                    j["AGVCurrTime"] = (agv->getTravelingTime()+agv->getTotalStopTime())*(timeRatio);
                                }
                            }

                        }
                        
                    }
                    
                }
            }

            //print j to console
            if (jsonOutput == 1)
            {
                cout << j.dump(4) << endl;
            }

            JsonOutput << j.dump(4) << endl;
            JsonOutput.close();
            
        }
    }

}

// Save state
json Utility::SaveState(std::vector<AGV *> agvs, std::vector<Agent *> agents,
                        float time, int timeline_pointer)
{
    /*structure of json file
    [
        {
            "time": 0,
            "agvs": [
                {
                    "id": 0,
                    "position": [0, 0],
                    "velocity": 1.2,
                    "direction": [0, 0]
                }
            ],
            "agents": [
                {
                    "id": 0,
                    "position": [0, 0],
                    "velocity": 1.2,
                    "direction": [0, 0]
                }
            ]
        }
    ]
    */
    json j;
    j["event_time"] = time+(float)(timeline_pointer*1000.0f);
    std::vector<json> agvList;
    for (AGV *agv : agvs)
    {
        json agvJson;
        agvJson["id"] = agv->getId();
        
        // position
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agvJson["position"] = {
            agv->getPosition().x,
            agv->getPosition().y
        };

        // velocity(Vector3f)
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agvJson["velocity"] = {
            agv->getVelocity().x,
            agv->getVelocity().y,
            agv->getVelocity().z
        };

        // direction(Vector3f)
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agvJson["direction"] = {
            agv->getDirection().x,
            agv->getDirection().y
        };

        // General Direction(int)
        agvJson["generalDirection"] = agv->getGeneralDirection();

        // Reach Destination(bool)
        agvJson["reachDestination"] = agv->getReachDestination();

        // travelingTime(int)
        agvJson["travelingTime"] = agv->getTravelingTime();

        // Width(float) and Length(float)
        agvJson["Width & Length"] = {
            agv->getWidth(),
            agv->getLength()
        };

        // acceleration(float)
        agvJson["acceleration"] = agv->getAcceleration();

        // numOfCollision(int)
        agvJson["numOfCollision"] = agv->getNumOfCollision();

        // thresholdDisToPedes(float)
        agvJson["thresholdDisToPedes"] = agv->getThresholdDisToPedes();

        // totalStopTime(int)
        agvJson["totalStopTime"] = agv->getTotalStopTime();

        // collisionStartTime(int)
        agvJson["collisionStartTime"] = agv->getCollisionStartTime();

        agvList.push_back(agvJson);
    }

    std::vector<json> agentList;
    for (Agent *agent : agents)
    {
        json agentJson;
        agentJson["id"] = agent->getId();
        
        // color
        /*
        {
            "red": 0.0, need to be multiplied by 255
            "green": 0.0,
            "blue": 0.0
        }
        */
        agentJson["color"] = {
            agent->getColor().x*255,
            agent->getColor().y*255,
            agent->getColor().z*255
        };

        // position
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agentJson["position"] = {
            agent->getPosition().x, 
            agent->getPosition().y
        };

        // desiredSpeed
        agentJson["desiredSpeed"] = agent->getDesiredSpeed();

        // velocity(Vector3f)
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agentJson["velocity"] = {
            agent->getVelocity().x,
            agent->getVelocity().y,
            agent->getVelocity().z
        };

        // destination(Point3f)
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agentJson["destination"] = {
            agent->getDestination().x,
            agent->getDestination().y
        };

        // position(Point3f)
        /*
        {
            "x": 0.0,
            "y": 0.0
        }
        */
        agentJson["position"] = {
            agent->getPosition().x,
            agent->getPosition().y
        };

        // isMoving(bool)
        agentJson["isMoving"] = agent->getIsMoving();

        // path(std::deque<Waypoint>)
        /*
        [
            {
                "position": {
                    "x": 0.0,
                    "y": 0.0
                },
                "radius": 0.0
            }
        ]
        */
        std::vector<json> pathList;
        for (Waypoint waypoint : agent->getFullPath())
        {
            json waypointJson;
            waypointJson["position"] = {
                waypoint.position.x,
                waypoint.position.y
            };
            waypointJson["radius"] = waypoint.radius;
            pathList.push_back(waypointJson);
        }
        agentJson["path"] = pathList;

        // radius(float)
        agentJson["radius"] = agent->getRadius();

        // impatient(float)
        agentJson["impatient"] = agent->getImpatient();

        // stopAtCorridor(float)
        agentJson["stopAtCorridor"] = agent->getStopAtCorridor();

        agentList.push_back(agentJson);

    }

    j["agvs"] = agvList;
    j["agents"] = agentList;

    return j;
    
}

// write state
void Utility::writeState(const char *fileName, std::vector<json> stateList, int timeline_pointer)
{

    // check for file exist
    json old_data;
    if (std::filesystem::exists(fileName))
    {
        // if file is not empty
        if (std::filesystem::file_size(fileName) != 0){
            
            // read the file
            std::ifstream f(fileName);
            old_data = json::parse(f);
            // clean up file
            std::ofstream ofs;
            ofs.open(fileName, std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            f.close();
        }
    }

    //sort the stateList by event_time ascending order
    // std::sort(stateList.begin(), stateList.end(), [](json a, json b) {
    //     return a["event_time"] < b["event_time"];
    // });

    // remove some event_time only take event_time by second instead of by millisecond
    int threshold = 1000;
    std::vector<json> stateListTemp;
    for (json &state : stateList)
    {
        // extra function to remove the completed agv
        for (int i = 0; i < state["agvs"].size(); i++)
        {
            if (state["agvs"][i]["reachDestination"] == true)
            {
                state["agvs"].erase(state["agvs"].begin() + i);
            }
        }
        json temp = state;
        temp["event_time"] = (int)temp["event_time"] / threshold;
        stateListTemp.push_back(temp);
    }
    stateList.clear();

    
    for (int i = 0; i <= stateListTemp[stateListTemp.size()-1]["event_time"]; i++)
    {
        for (json &state : stateListTemp)
        {
            if (state["event_time"] == i)
            {
                stateList.push_back(state);
                break;
            }
        }
        
    }

    // remove duplicate event_time start from the first event in the list(the event which has the lowest event_time)
    // loop through old_data["timeline"] and remove the duplicate event_time
    int deleteEventTime = timeline_pointer;
    
    // new stateListTemp
    std::vector<json> stateListTempNew;

    for (json &state : old_data["timeline"])
    {
        if (state["event_time"] < deleteEventTime)
        {
            stateListTempNew.push_back(state);
        }
    }

    // clean up file
    std::ofstream ofs;
    ofs.open(fileName, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    ofstream output(fileName, ios::app);

    // add the new data to the old data
    // clean up old_data["timeline"]
    old_data["timeline"] = stateListTempNew;
    for (json &state : stateList)
    {
        old_data["timeline"].push_back(state);
    }
    
    output << old_data.dump(4) << endl;
    output.close();

    // // debug output
    // ofstream debugOutput("data/debug.json", ios::app);
    // json debug;
    // debug["timeline"] = stateList;
    // debugOutput << debug.dump(4) << endl;
    // debugOutput.close();

    // clean up data
    stateList.clear();
    stateListTemp.clear();
    old_data.clear();
    stateListTempNew.clear();
}

// timeline controller
void Utility::timeline_writer(
        const char *fileName,
        const char *arcID,
        int start_time,
        int end_time,
        vector<int> agvIDs
)
{
    json j;
    j["arcID"] = arcID;
    j["start_time"] = start_time;
    j["end_time"] = end_time/1000;
    j["agvIDs"] = agvIDs;

    // load the timeline data then append to the timeline
    // but first check if the file exist
    if (!std::filesystem::exists(fileName))
    {
        // create the file
        ofstream output(fileName, ios::app);
        json data;
        // data is a list ["timeline": []]
        data["timeline"] = {};
        output << data.dump(4) << endl;
        output.close();
        }

    // load the timeline data
    std::ifstream f(fileName);
    json data = json::parse(f);
    f.close();

    // struct: 
    /*
    {
        'timeline': [
            {
            'arcID': 'arc1',
            'start_time': 0,
            'end_time': 1000
            },
            {
            'arcID': 'arc1',
            'start_time': 1000,
            'end_time': 2000
            }

        ]
    }
    */

    // remove the data that has start_time >= new start_time and same junction_name and append the new timeline data
    std::vector<json> timelineTemp;
    for (json &timeline : data["timeline"])
    {
        if (timeline["arcID"] != arcID || timeline["start_time"] < start_time)
        {
            timelineTemp.push_back(timeline);
        }
    }
    timelineTemp.push_back(j);
    data["timeline"] = timelineTemp;

    // write the timeline data back to the file
    // clean up file
    std::ofstream ofs;
    ofs.open(fileName, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    
    ofstream output(fileName, ios::app);
    output << data.dump(4) << endl;
    output.close();
    
}

int Utility::timeline_getter(const char *fileName)
{
    // load the timeline data
    std::ifstream f(fileName);
    json data = json::parse(f);
    f.close();

    // get the most recent timeline data without caring about the arcID
    // this mean get the highest end_time
    
    int max = 0;
    for (json &timeline : data["timeline"])
    {
        if (timeline["end_time"] > max)
        {
            max = timeline["end_time"];
        }
    }
    cout << "The most recent timeline data: " << max << endl;
    return max;
}


// calculate number of people in each flow
std::vector<int> Utility::getNumPedesInFlow(int junctionType,
                                            int totalPedestrian)
{
    int numFlow = 0;
    if (junctionType == 2)
    {
        numFlow = 6;
    }

    std::vector<int> v(numFlow, 0);
    int j = 0;
    for (int i = 0; i < totalPedestrian; i++)
    {
        v[j] = v[j] + 1;
        j = j + 1;
        if (j == numFlow)
        {
            j = 0;
        }
    }
    return v;
}

// get list velocity of all pedestrians: type 0 - Discrete distribution, type 1
// - T distribution
std::vector<double> Utility::getPedesVelocity(int type, json inputData,
                                              float deviationParam)
{
    
    return getPedesVelocityBasedDDis(inputData, deviationParam);
    
}

std::vector<double> Utility::getPedesVelocityBasedDDis(json inputData,
                                                       float deviationParam)
{
    vector<double> v;
    float perNoDisabilityWithoutOvertaking =
        float(inputData["p1"]["value"]) * deviationParam;
    float perNoDisabilityWithOvertaking =
        float(inputData["p2"]["value"]) * deviationParam;
    float perWalkingWithCrutches =
        float(inputData["p3"]["value"]) * deviationParam;
    float perWalkingWithSticks = float(inputData["p4"]["value"]) * deviationParam;
    float perWheelchairs = float(inputData["p5"]["value"]) * deviationParam;
    // float perTheBlind = inputData["p6"]["value"];
    float perTheBlind =
        100 - (perNoDisabilityWithoutOvertaking + perNoDisabilityWithOvertaking +
               perWalkingWithCrutches + perWalkingWithSticks + perWheelchairs);

    const int nrolls = 10000; // number of experiments
    const int numPedes =
        int(int(inputData["numOfAgents"]["value"]) *
            deviationParam); // maximum number of pedes to distribute

    std::default_random_engine generator;
    std::discrete_distribution<int> distribution{perNoDisabilityWithoutOvertaking,
                                                 perNoDisabilityWithOvertaking,
                                                 perWalkingWithCrutches,
                                                 perWalkingWithSticks,
                                                 perWheelchairs,
                                                 perTheBlind};

    int p[6] = {};

    for (int i = 0; i < nrolls; ++i)
    {
        int number = distribution(generator);
        ++p[number];
    }

    std::map<int, float> map;
    map[0] = V1;
    map[1] = V2;
    map[2] = V3;
    map[3] = V4;
    map[4] = V5;
    map[5] = V6;

    for (int i = 0; i < 6; ++i)
    {
        // std::cout << i << ": " << p[i] * numPedes / nrolls << std::endl;
        // std::cout << i << ": " << std::string(p[i] * numPedes / nrolls, '*') <<
        // std::endl;
        for (int j = 0; j < p[i] * numPedes / nrolls; j++)
        {
            v.push_back(map[i]);
        }
    }
    int curSize = v.size();
    for (int i = 0; i < numPedes - curSize; i++)
    {
        v.push_back(map[0]);
    }

    return v;
}

std::vector<float> Utility::getWallCoordinates(float walkwayWidth,
                                               std::vector<float> juncData)
{
    std::vector<float> v;

    std::vector<float> mapLimit = getMapLimit(walkwayWidth, juncData);

    float temp = walkwayWidth / 2;

    float leftWidthLimit = mapLimit[0];
    float rightWidthLimit = mapLimit[1];

    float lowerHeightLimit = mapLimit[2];
    float upperHeightLimit = mapLimit[3];

    if (juncData.size() == 2)
    {
        // Upper Wall
        v.insert(v.end(), {leftWidthLimit, temp, rightWidthLimit, temp});
        // Lower Wall
        v.insert(v.end(), {leftWidthLimit, -temp, rightWidthLimit, -temp});
        return v;
    }

    // Upper Wall
    v.insert(v.end(), {leftWidthLimit, temp, -temp, temp});
    v.insert(v.end(), {temp, temp, rightWidthLimit, temp});
    // Lower Wall
    v.insert(v.end(), {leftWidthLimit, -temp, -temp, -temp});
    v.insert(v.end(), {temp, -temp, rightWidthLimit, -temp});
    // Left Wall
    v.insert(v.end(), {-temp, upperHeightLimit, -temp, temp});
    v.insert(v.end(), {-temp, -temp, -temp, lowerHeightLimit});
    // Right Wall
    v.insert(v.end(), {temp, upperHeightLimit, temp, temp});
    v.insert(v.end(), {temp, -temp, temp, lowerHeightLimit});

    return v;
}

// Convert miliseconds to pretty form
std::string Utility::convertTime(int ms)
{
    //cout << "ms: " << ms << endl;
    // 3600000 milliseconds in an hour
    long hr = ms / 3600000;
    ms = ms - 3600000 * hr;

    // 60000 milliseconds in a minute
    long min = ms / 60000;
    ms = ms - 60000 * min;

    // 1000 milliseconds in a second
    long sec = ms / 1000;
    ms = ms - 1000 * sec;

    return std::to_string(hr) + std::string("h ") + std::to_string(min) +
           std::string("m ") + std::to_string(sec) + std::string("s ") +
           std::to_string(ms) + std::string("ms");
}

// Position 0/1/2/3: Left/Right/Lower/Upper Limit
std::vector<float> Utility::getMapLimit(float walkwayWidth,
                                        std::vector<float> juncData)
{
    std::vector<float> v;
    float leftWidthLimit = -1;
    float rightWidthLimit = -1;
    float lowerHeightLimit = -1;
    float upperHeightLimit = -1;
    // float leftWidthLimit = 0;
    // float rightWidthLimit = 0;
    // float lowerHeightLimit = 0;
    // float upperHeightLimit = 0;

    float temp = walkwayWidth / 2;

    if (juncData.size() == 2)
    {
        leftWidthLimit = -juncData[0] - temp;
        rightWidthLimit = juncData[1] + temp;
    }
    else
    {
        leftWidthLimit = -juncData[0] - temp;
        rightWidthLimit = juncData[2] + temp;

        lowerHeightLimit = -juncData[1] - temp;
        upperHeightLimit = juncData[3] + temp;
    }

    v.insert(v.end(), {leftWidthLimit, rightWidthLimit, lowerHeightLimit,
                       upperHeightLimit});

    return v;
}

// direction: 0 To Right, 1 To Left, 2 To Bottom, 3 To Top
// side: 0 Left side, 1 Center, 2 Right side
std::vector<float> Utility::getPedesDestination(int direction, int side,
                                                float walkwayWidth,
                                                std::vector<float> juncData,
                                                bool stopAtCorridor)
{
    std::vector<float> v;

    std::vector<float> mapLimit = getMapLimit(walkwayWidth, juncData);
    float leftWidthLimit = mapLimit[0];
    float rightWidthLimit = mapLimit[1];
    float lowerHeightLimit = mapLimit[2];
    float upperHeightLimit = mapLimit[3];

    float radius = 0.5;
    switch (direction)
    {
        // To Right
    case 0:
    {
        float latitude;
        if (stopAtCorridor)
        {
            // latitude = Utility::randomFloat(0, rightWidthLimit - 2);
            latitude = Utility::randomInt(leftWidthLimit + 2, rightWidthLimit - 2);
        }
        else
        {
            latitude =
                Utility::randomFloat(rightWidthLimit + 1, rightWidthLimit + 2);
        }
        switch (side)
        {
        case 0:
        {
            v.insert(v.end(),
                     {latitude,
                      //    Utility::randomFloat(walkwayWidth / 2 - walkwayWidth /
                      //    3, walkwayWidth / 2),
                      Utility::randomFloat(-walkwayWidth / 2, walkwayWidth / 2),
                      radius});
            return v;
            break;
        }
        case 1:
        {
            v.insert(v.end(),
                     {latitude,
                      Utility::randomFloat(-walkwayWidth / 2 + walkwayWidth / 3,
                                           walkwayWidth / 2 - walkwayWidth / 3),
                      radius});
            return v;
            break;
        }
        case 2:
        {
            v.insert(v.end(),
                     {latitude,
                      Utility::randomFloat(-walkwayWidth / 2,
                                           -walkwayWidth / 2 + walkwayWidth / 3),
                      radius});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
        // To Left
    case 1:
    {
        float latitude;
        if (stopAtCorridor)
        {
            latitude = Utility::randomFloat(leftWidthLimit + 2, 0);
        }
        else
        {
            latitude = Utility::randomFloat(leftWidthLimit - 2, leftWidthLimit - 1);
        }
        switch (side)
        {
        case 0:
        {
            v.insert(v.end(),
                     {latitude,
                      Utility::randomFloat(-walkwayWidth / 2,
                                           -walkwayWidth / 2 + walkwayWidth / 3),
                      radius});
            return v;
            break;
        }
        case 1:
        {
            v.insert(v.end(),
                     {latitude,
                      Utility::randomFloat(-walkwayWidth / 2 + walkwayWidth / 3,
                                           walkwayWidth / 2 - walkwayWidth / 3),
                      radius});
            return v;
            break;
        }
        case 2:
        {
            v.insert(v.end(),
                     {latitude,
                      Utility::randomFloat(walkwayWidth / 2 - walkwayWidth / 3,
                                           walkwayWidth / 2),
                      radius});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
        // To Bottom
    case 2:
    {
        float longitude;
        if (stopAtCorridor)
        {
            longitude = Utility::randomFloat(lowerHeightLimit + 2, 0);
        }
        else
        {
            longitude =
                Utility::randomFloat(lowerHeightLimit - 2, lowerHeightLimit - 1);
        }
        switch (side)
        {
        case 0:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(walkwayWidth / 2 - walkwayWidth / 3,
                                           walkwayWidth / 2),
                      longitude, radius});
            return v;
            break;
        }
        case 1:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(-walkwayWidth / 2 + walkwayWidth / 3,
                                           walkwayWidth / 2 - walkwayWidth / 3),
                      longitude, radius});
            return v;
            break;
        }
        case 2:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(-walkwayWidth / 2,
                                           -walkwayWidth / 2 + walkwayWidth / 3),
                      longitude, radius});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
        // To Top
    case 3:
    {
        float longitude;
        if (stopAtCorridor)
        {
            longitude = Utility::randomFloat(0, upperHeightLimit - 2);
        }
        else
        {
            longitude =
                Utility::randomFloat(upperHeightLimit + 1, upperHeightLimit + 2);
        }
        switch (side)
        {
        case 0:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(-walkwayWidth / 2,
                                           -walkwayWidth / 2 + walkwayWidth / 3),
                      longitude, radius});
            return v;
            break;
        }
        case 1:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(-walkwayWidth / 2 + walkwayWidth / 3,
                                           walkwayWidth / 2 - walkwayWidth / 3),
                      longitude, radius});
            return v;
            break;
        }
        case 2:
        {
            v.insert(v.end(),
                     {Utility::randomFloat(walkwayWidth / 2 - walkwayWidth / 3,
                                           walkwayWidth / 2),
                      longitude, radius});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        return v;
        break;
    }
    return v;
}

// direction: 0 From Left, 1 From Right, 2 From Top, 3 From Bottom
std::vector<float> Utility::getPedesSource(int direction, float totalLength,
                                           float subLength, float caravanWidth,
                                           float walkwayWidth,
                                           std::vector<float> juncData)
{
    std::vector<float> v;
    float totalArea = totalLength * caravanWidth;
    float centerLength = totalLength - 2 * subLength;

    std::mt19937 gen(std::random_device{}());
    std::vector<double> chances{subLength * caravanWidth / totalArea,
                                centerLength * caravanWidth / totalArea,
                                subLength * caravanWidth / totalArea};
    // Initialize to same length.
    std::vector<int> choices(chances.size());
    choices = {0, 1, 2};
    // size_t is suitable for indexing.
    std::discrete_distribution<std::size_t> d{chances.begin(), chances.end()};

    int sampled_value = choices[d(gen)];
    // cout << "sampled_value " << sampled_value << endl;

    std::vector<float> mapLimit = getMapLimit(walkwayWidth, juncData);
    float rightWidthLimit = mapLimit[1];
    float upperHeightLimit = mapLimit[3];

    float horLandmark = rightWidthLimit;
    float verLandmark = upperHeightLimit;
    if (totalLength > 40)
    {
        horLandmark = totalLength / 2;
        verLandmark = totalLength / 2;
    }
    // Calculate coordinates from Head to Tail of each Caravan
    switch (direction)
    {
    // From Left to Right
    case 0:
        switch (sampled_value)
        {
        case 0:
            v.insert(v.end(),
                     {Utility::randomFloat(-horLandmark + centerLength / 2,
                                           -horLandmark + totalLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        case 1:
            v.insert(v.end(),
                     {Utility::randomFloat(-horLandmark - centerLength / 2,
                                           -horLandmark + centerLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        case 2:
            v.insert(v.end(),
                     {Utility::randomFloat(-horLandmark - totalLength / 2,
                                           -horLandmark - centerLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        default:
            break;
        }
        break;

        // From Right to Left
    case 1:
        switch (sampled_value)
        {
        case 0:
            v.insert(v.end(),
                     {Utility::randomFloat(horLandmark - totalLength / 2,
                                           horLandmark - centerLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        case 1:
            v.insert(v.end(),
                     {Utility::randomFloat(horLandmark - centerLength / 2,
                                           horLandmark + centerLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        case 2:
            v.insert(v.end(),
                     {Utility::randomFloat(horLandmark + centerLength / 2,
                                           horLandmark + totalLength / 2),
                      Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2)});
            return v;
            break;
        default:
            break;
        }
        break;

        // From Top to Bottom
    case 2:
        switch (sampled_value)
        {
        case 0:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(verLandmark - totalLength / 2,
                                           verLandmark - centerLength / 2)});
            return v;
            break;
        case 1:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(verLandmark - centerLength / 2,
                                           verLandmark + centerLength / 2)});
            return v;
            break;
        case 2:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(verLandmark + centerLength / 2,
                                           verLandmark + totalLength / 2)});
            return v;
            break;
        default:
            break;
        }
        break;

        // From Bottom to Top
    case 3:
        switch (sampled_value)
        {
        case 0:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(-verLandmark + centerLength / 2,
                                           -verLandmark + totalLength / 2)});
            return v;
            break;
        case 1:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(-verLandmark - centerLength / 2,
                                           -verLandmark + centerLength / 2)});
            return v;
            break;
        case 2:
            v.insert(v.end(),
                     {Utility::randomFloat(-caravanWidth / 2, caravanWidth / 2),
                      Utility::randomFloat(-verLandmark - totalLength / 2,
                                           -verLandmark - centerLength / 2)});
            return v;
            break;
        default:
            break;
        }
        break;
    default:
        return v;
        break;
    }
    return v;
}

std::vector<float> Utility::getPedesColor(float maxSpeed, float minSpeed,
                                          float desiredSpeed, int type)
{
    std::vector<float> v;

    if (type == 1)
    {
        float oneThirdVeloRange = (maxSpeed - minSpeed) / 3;

        if (desiredSpeed >= MEAN)
        {
            v.insert(v.end(), {GREEN.x, GREEN.y, GREEN.z}); // Green
            return v;
        }
        else if (desiredSpeed < MEAN &&
                 desiredSpeed >= minSpeed + oneThirdVeloRange)
        {
            v.insert(v.end(), {BLACK.x, BLACK.y, BLACK.z}); // Black
            return v;
        }
        else
        {
            v.insert(v.end(), {RED.x, RED.y, RED.z}); // Red
            return v;
        }
    }
    else
    {
        // cout << fabs(V1 - desiredSpeed)  << endl;
        if (fabs(V1 - desiredSpeed) < 0.01)
        {
            v.insert(v.end(), {GREEN.x, GREEN.y, GREEN.z});
            return v;
        }
        else if (fabs(V2 - desiredSpeed) < 0.01)
        {
            v.insert(v.end(), {PURPLE.x, PURPLE.y, PURPLE.z});
            return v;
        }
        else if (fabs(V3 - desiredSpeed) < 0.01)
        {
            v.insert(v.end(), {RED.x, RED.y, RED.z});
            return v;
        }
        else if (fabs(V4 - desiredSpeed) < 0.01)
        {
            v.insert(v.end(), {WOOD.x, WOOD.y, WOOD.z});
            return v;
        }
        else if (fabs(V5 - desiredSpeed) < 0.01)
        {
            v.insert(v.end(), {GRAY.x, GRAY.y, GRAY.z});
            return v;
        }
        else
        {
            v.insert(v.end(), {BLACK.x, BLACK.y, BLACK.z});
            return v;
        }
    }
}

float getCoor(float x, float verAsymtote, float horAsymtote)
{
    return horAsymtote * x / (x - verAsymtote);
}

std::vector<Point3f> Utility::getRouteAGV(int src, int turningDirection,
                                          float walkwayWidth,
                                          std::vector<float> juncData)
{
    std::vector<Point3f> v;
    int junctionType = juncData.size();
    if (junctionType == 2)
    {
        v = getRouteAGVHallway(src, walkwayWidth, juncData);
    }
    return v;
}

// src = {0, 1, 2} ~ Go from Left, Bottom, Right side
std::vector<Point3f> Utility::getRouteAGVHallway(int src, float walkwayWidth,
                                                 std::vector<float> juncData)
{
    std::vector<Point3f> v;

    float leftWidthLimit = -juncData[0] - walkwayWidth / 2;
    float rightWidthLimit = juncData[1] + walkwayWidth / 2;

    if (src == 0)
    {
        // v.insert(v.end(), {Point3f(leftWidthLimit - 1, -walkwayWidth / 3, 0.0),
        //                    Point3f(rightWidthLimit + 1, -walkwayWidth / 3, 0.0)});
        // v.insert(v.end(), {Point3f(rightWidthLimit + 2, -walkwayWidth / 3, 0.0)});
        v.insert(v.end(), {Point3f(leftWidthLimit + 1, -walkwayWidth / 3, 0.0),
                           Point3f(rightWidthLimit - 1, -walkwayWidth / 3, 0.0)});
        v.insert(v.end(), {Point3f(rightWidthLimit - 1, -walkwayWidth / 3, 0.0)});
        return v;
    }
    else
    {
        v.insert(v.end(), {Point3f(rightWidthLimit - 1, walkwayWidth / 3, 0.0),
                           Point3f(leftWidthLimit + 1, walkwayWidth / 3, 0.0)});
        v.insert(v.end(), {Point3f(leftWidthLimit + 1, walkwayWidth / 3, 0.0)});
        return v;
    }
    std::cout << src << std::endl;

    return v;
}

// src = {0, 1, 2} ~ Go from Left, Bottom, Right side
// turningDirection = {0, 1, 2} - Turn Left, Go Straight, Turn Right
std::vector<Point3f> Utility::getRouteAGVTJunction(
    int src, int turningDirection, float walkwayWidth,
    std::vector<float> juncData)
{
    std::vector<Point3f> v;

    float horWalkwayWidth = walkwayWidth;
    float verWalkwayWidth = walkwayWidth;

    float posVerAsymtote = verWalkwayWidth / 2;
    float negVerAsymtote = -verWalkwayWidth / 2;

    float posHorAsymtote = horWalkwayWidth / 2;
    // float negHorAsymtote = -horWalkwayWidth / 2;

    float leftWidthLimit = -juncData[0] - walkwayWidth / 2;
    float rightWidthLimit = juncData[2] + walkwayWidth / 2;

    float lowerHeightLimit = -juncData[1] - walkwayWidth / 2;

    switch (src)
    {
        // Go From Left side
    case 0:
    {
        switch (turningDirection)
        {
        case 0:
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(leftWidthLimit - 1, -horWalkwayWidth / 3, 0.0),
                      Point3f(rightWidthLimit + 1, -horWalkwayWidth / 3, 0.0)});
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 2, -horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
            // Turn Right
        case 2:
        {
            v.insert(
                v.end(),
                {Point3f(leftWidthLimit - 1, -horWalkwayWidth / 3, 0.0),
                 Point3f(-verWalkwayWidth / 2, -horWalkwayWidth / 3, 0.0),
                 Point3f(-verWalkwayWidth / 3.5, -horWalkwayWidth / 2.5, 0.0),
                 Point3f(-verWalkwayWidth / 3.5, -horWalkwayWidth / 2, 0.0)});
            v.insert(v.end(),
                     {Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0),
                      Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 2, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
    // Go From Bottom side
    case 1:
    {
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = verWalkwayWidth / 3; x >= leftWidthLimit - 1; x--)
            {
                float y = getCoor(x, posVerAsymtote, posHorAsymtote);
                if (y > horWalkwayWidth / 3)
                {
                    y = horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.front();
            if (checker.y > lowerHeightLimit)
            {
                v.insert(v.begin(), Point3f(checker.x, lowerHeightLimit - 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
            // Turn Right
        case 2:
        {
            v.insert(v.end(),
                     {Point3f(verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0),
                      Point3f(verWalkwayWidth / 3, -horWalkwayWidth / 2, 0.0),
                      Point3f(verWalkwayWidth / 2.5, -horWalkwayWidth / 3.5, 0.0),
                      Point3f(verWalkwayWidth / 2, -horWalkwayWidth / 3.5, 0.0)});
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 1, -horWalkwayWidth / 3, 0.0),
                      Point3f(rightWidthLimit + 2, -horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    }
    // Go from Right side
    case 2:
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = rightWidthLimit + 1; x >= -verWalkwayWidth / 3; x--)
            {
                float y = getCoor(x, negVerAsymtote, posHorAsymtote);
                if (y > horWalkwayWidth / 3)
                {
                    y = horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.back();
            if (checker.y > lowerHeightLimit)
            {
                v.push_back(Point3f(checker.x, lowerHeightLimit - 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 1, horWalkwayWidth / 3, 0.0),
                      Point3f(leftWidthLimit - 1, horWalkwayWidth / 3, 0.0)});
            v.insert(v.end(),
                     {Point3f(leftWidthLimit - 2, horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    default:
        break;
    }

    return v;
}

// src = {0, 1, 2, 3} ~ Go from Left, Bottom, Right, Top side
// turningDirection = {0, 1, 2} - Turn Left, Go Straight, Turn Right
std::vector<Point3f> Utility::getRouteAGVCrossRoad(
    int src, int turningDirection, float walkwayWidth,
    std::vector<float> juncData)
{
    float horWalkwayWidth = walkwayWidth;
    float verWalkwayWidth = walkwayWidth;

    float posVerAsymtote = verWalkwayWidth / 2;
    float negVerAsymtote = -verWalkwayWidth / 2;

    float posHorAsymtote = horWalkwayWidth / 2;
    float negHorAsymtote = -horWalkwayWidth / 2;

    float leftWidthLimit = -juncData[0] - walkwayWidth / 2;
    float rightWidthLimit = juncData[2] + walkwayWidth / 2;

    float lowerHeightLimit = -juncData[1] - walkwayWidth / 2;
    float upperHeightLimit = juncData[3] + walkwayWidth / 2;

    std::vector<Point3f> v;
    switch (src)
    {
    // Go from Left side
    case 0:
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = leftWidthLimit - 1; x <= verWalkwayWidth / 3; x++)
            {
                float y = getCoor(x, posVerAsymtote, negHorAsymtote);
                if (y < -horWalkwayWidth / 3)
                {
                    y = -horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.back();
            if (checker.y < upperHeightLimit)
            {
                v.push_back(Point3f(checker.x, upperHeightLimit + 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(leftWidthLimit - 1, -horWalkwayWidth / 3, 0.0),
                      Point3f(rightWidthLimit + 1, -horWalkwayWidth / 3, 0.0)});
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 2, -horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
            // Turn Right
        case 2:
        {
            v.insert(
                v.end(),
                {Point3f(leftWidthLimit - 1, -horWalkwayWidth / 3, 0.0),
                 Point3f(-verWalkwayWidth / 2, -horWalkwayWidth / 3, 0.0),
                 Point3f(-verWalkwayWidth / 3.5, -horWalkwayWidth / 2.5, 0.0),
                 Point3f(-verWalkwayWidth / 3.5, -horWalkwayWidth / 2, 0.0)});
            v.insert(v.end(),
                     {Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0),
                      Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 2, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;

        // Go from Bottom side
    case 1:
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = verWalkwayWidth / 3; x >= leftWidthLimit - 1; x--)
            {
                float y = getCoor(x, posVerAsymtote, posHorAsymtote);
                if (y > horWalkwayWidth / 3)
                {
                    y = horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.front();
            if (checker.y > lowerHeightLimit)
            {
                v.insert(v.begin(), Point3f(checker.x, lowerHeightLimit - 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0),
                      Point3f(verWalkwayWidth / 3, upperHeightLimit + 1, 0.0)});
            v.insert(v.end(),
                     {Point3f(verWalkwayWidth / 3, upperHeightLimit + 2, 0.0)});
            return v;
            break;
        }
            // Turn Right
        case 2:
        {
            v.insert(v.end(),
                     {Point3f(verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0),
                      Point3f(verWalkwayWidth / 3, -horWalkwayWidth / 2, 0.0),
                      Point3f(verWalkwayWidth / 2.5, -horWalkwayWidth / 3.5, 0.0),
                      Point3f(verWalkwayWidth / 2, -horWalkwayWidth / 3.5, 0.0)});
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 1, -horWalkwayWidth / 3, 0.0),
                      Point3f(rightWidthLimit + 2, -horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;

        // Go from Right side
    case 2:
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = rightWidthLimit + 1; x >= -verWalkwayWidth / 3; x--)
            {
                float y = getCoor(x, negVerAsymtote, posHorAsymtote);
                if (y > horWalkwayWidth / 3)
                {
                    y = horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.back();
            if (checker.y > lowerHeightLimit)
            {
                v.push_back(Point3f(checker.x, lowerHeightLimit - 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 1, horWalkwayWidth / 3, 0.0),
                      Point3f(leftWidthLimit - 1, horWalkwayWidth / 3, 0.0)});
            v.insert(v.end(),
                     {Point3f(leftWidthLimit - 2, horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
            // Turn Right
        case 2:
        {
            v.insert(v.end(),
                     {Point3f(rightWidthLimit + 1, horWalkwayWidth / 3, 0.0),
                      Point3f(verWalkwayWidth / 2, horWalkwayWidth / 3, 0.0),
                      Point3f(verWalkwayWidth / 3.5, horWalkwayWidth / 2.5, 0.0),
                      Point3f(verWalkwayWidth / 3.5, horWalkwayWidth / 2, 0.0)});
            v.insert(v.end(),
                     {Point3f(verWalkwayWidth / 3, upperHeightLimit + 1, 0.0),
                      Point3f(verWalkwayWidth / 3, upperHeightLimit + 2, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;

        // Go from Top side
    case 3:
        switch (turningDirection)
        {
        // Turn Left
        case 0:
        {
            for (int x = -verWalkwayWidth / 3; x <= rightWidthLimit + 1; x++)
            {
                float y = getCoor(x, negVerAsymtote, negHorAsymtote);
                if (y < -horWalkwayWidth / 3)
                {
                    y = -horWalkwayWidth / 3;
                }
                v.push_back(Point3f(x, y, 0.0));
            }
            Point3f checker = v.front();
            if (checker.y < upperHeightLimit)
            {
                v.insert(v.begin(), Point3f(checker.x, upperHeightLimit + 1, 0.0));
            }
            return v;
            break;
        }
            // Go Straight
        case 1:
        {
            v.insert(v.end(),
                     {Point3f(-verWalkwayWidth / 3, upperHeightLimit + 1, 0.0),
                      Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 1, 0.0)});
            v.insert(v.end(),
                     {Point3f(-verWalkwayWidth / 3, lowerHeightLimit - 2, 0.0)});
            return v;
            break;
        }
            // Turn Right
        case 2:
        {
            v.insert(v.end(),
                     {Point3f(-verWalkwayWidth / 3, upperHeightLimit + 1, 0.0),
                      Point3f(-verWalkwayWidth / 3, horWalkwayWidth / 2, 0.0),
                      Point3f(-verWalkwayWidth / 2.5, horWalkwayWidth / 3.5, 0.0),
                      Point3f(-verWalkwayWidth / 2, horWalkwayWidth / 3.5, 0.0)});
            v.insert(v.end(),
                     {Point3f(leftWidthLimit - 1, horWalkwayWidth / 3, 0.0),
                      Point3f(leftWidthLimit - 2, horWalkwayWidth / 3, 0.0)});
            return v;
            break;
        }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return v;
}

Point3f Utility::getIntermediateDes(Point3f position, float verWalkwayWidth,
                                    float horWalkwayWidth)
{
    if (position.x > 0 && position.y > 0)
    {
        return Point3f(verWalkwayWidth / 3, horWalkwayWidth / 3, 0.0);
    }
    else if (position.x < 0 && position.y > 0)
    {
        return Point3f(-verWalkwayWidth / 3, horWalkwayWidth / 3, 0.0);
    }
    else if (position.x < 0 && position.y < 0)
    {
        return Point3f(-verWalkwayWidth / 3, -horWalkwayWidth / 3, 0.0);
    }
    else
    {
        return Point3f(verWalkwayWidth / 3, -horWalkwayWidth / 3, 0.0);
    }
}

bool Utility::isPositionErr(Point3f position, float hallwayWidth,
                            int junctionType, std::vector<AGV *> agvs)
{
    float posLimit = hallwayWidth / 2;
    float negLimit = -hallwayWidth / 2;
    float x = position.x;
    float y = position.y;
    
    if (junctionType == 2)
    {
        if (y >= posLimit || y <= negLimit)
        {
            return true;
        }
    }

    for (AGV *agv : agvs)
    {
        if (!agv->getIsMoving())
        {
            continue;
        }
        float distance = (agv->getWidth() > agv->getLength()) ? agv->getWidth()
                                                              : agv->getLength();
        if (position.distance(agv->getPosition()) < distance / 2)
        {
            return true;
        }
    }

    return false;
}

int Utility::getNumAGVCompleted(std::vector<AGV *> agvs)
{
    int count = 0;
    for (AGV *agv : agvs)
    {
        if (!agv->getIsMoving() && agv->getTravelingTime() != 0)
        {
            count = count + 1;
        }
    }
    return count;
}

int Utility::randomInt(int from, int to)
{
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(from, to);
    return distr(gen);
}
