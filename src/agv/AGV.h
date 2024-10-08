#ifndef AGV_H
#define AGV_H

#include "lib/vecmath/vecmath.h"
#include <deque>
#include <vector>
#include "../object/MovingObject.h"

struct agvs_data
{
    //position
    Point3f position;
    //id
    int id;
    // reach destination
    bool reachDestination;
};

class AGV : public MovingObject
{
private:
    static int agvIdx;
    float width, length;
    int travelingTime;
    int numOfCollision;
    float acceleration;
    float thresholdDisToPedes;
    Point3f pointA, pointB, pointC, pointD;
    Vector3f direction;
    bool isCollision;
    int totalStopTime;
    int collisionStartTime;
    int generalDirection;
    bool reachDestination;

public:
    AGV();

    ~AGV();

    void setAgvIdx(int agvIdx);

    void setAgvSize(float width, float length);

    void setReachDestination(bool reachDestination);

    bool getReachDestination() const { return reachDestination; }

    float getWidth() const { return width; }

    float getLength() const { return length; }

    void setTravelingTime(int travelingTime);

    int getTravelingTime() const { return travelingTime; }

    void setNumOfCollision(int numOfCollision);

    int getNumOfCollision() const { return numOfCollision; }

    void setAcceleration(float acceleration);

    double getAcceleration() const { return acceleration; }

    void setThresholdDisToPedes(float thresholdDisToPedes);

    float getThresholdDisToPedes() const { return thresholdDisToPedes; }

    int getTotalStopTime() const { return totalStopTime; }

    void setTotalStopTime(int totalStopTime);

    int getCollisionStartTime() const { return collisionStartTime; }

    void setCollisionStartTime(int collisionStartTime);

    void setPoints(Point3f pointA, Point3f pointB, Point3f pointC, Point3f pointD);

    void setDirection(float x, float y);

    void setGeneralDirection(int generalDirection);

    Vector3f getDirection() const { return direction; }

    int getGeneralDirection() const { return generalDirection; }

    Point3f getNearestPoint(Point3f position_i);

    bool isNearPedes(std::vector<Point3f> position_list);

    // is near another AGV but the difference is the only stop when another AGV is blocking the way of this AGV
    bool isNearAGV(std::vector<agvs_data> position_list_agv);

    bool isNearAnyThing(std::vector<Point3f> position_list, std::vector<int> ids_list_agv, std::vector<Point3f> position_list_agv);

    using MovingObject::move;
    void move(float stepTime, std::vector<Point3f> position_list, std::vector<agvs_data> position_list_agv);
};

#endif
