#include "AGV.h"

using namespace std;

int AGV::agvIdx = -1;

AGV::AGV()
{
    MovingObject();
    id = 0;
    setAgvSize(0.35F, 0.75F);
    travelingTime = 0;
    numOfCollision = 0;
    thresholdDisToPedes = 0;
    isMoving = false;
    velocity.set(0.0, 0.0, 0.0);
    collisionStartTime = 0;
    totalStopTime = 0;
    generalDirection = 0;
    reachDestination = false;
}

AGV::~AGV()
{
    // agvIdx--;
}

void AGV::setAgvIdx(int agvIdx)
{
    this->agvIdx = agvIdx;
    this->id = agvIdx;
}

void AGV::setAgvSize(float width, float length)
{
    this->width = width;
    this->length = length;
}

void AGV::setReachDestination(bool reachDestination)
{
    this->reachDestination = reachDestination;
}

void AGV::setTravelingTime(int travelingTime)
{
    this->travelingTime = travelingTime;
}

void AGV::setNumOfCollision(int numOfCollision)
{
    this->numOfCollision = numOfCollision;
}

void AGV::setAcceleration(float acceleration)
{
    this->acceleration = acceleration;
}

void AGV::setThresholdDisToPedes(float thresholdDisToPedes)
{
    this->thresholdDisToPedes = thresholdDisToPedes;
}

void AGV::setTotalStopTime(int totalStopTime)
{
    this->totalStopTime = totalStopTime;
};

void AGV::setCollisionStartTime(int collisionStartTime)
{
    this->collisionStartTime = collisionStartTime;
};

void AGV::setPoints(Point3f pointA, Point3f pointB, Point3f pointC, Point3f pointD)
{
    this->pointA = pointA;
    this->pointB = pointB;
    this->pointC = pointC;
    this->pointD = pointD;
}

void AGV::setDirection(float x, float y)
{
    this->direction.set(x, y, 0.0F);
}

void AGV::setGeneralDirection(int generalDirection)
{
    this->generalDirection = generalDirection;
}

Point3f AGV::getNearestPoint(Point3f positionAgent)
{
    vector<Point3f> points;
    points.push_back(pointA);
    points.push_back(pointB);
    points.push_back(pointC);
    points.push_back(pointD);
    Vector3f relativeEnd, relativePos, relativeEndScal, relativePosScal;
    float dotProduct;
    vector<Point3f> nearestPoint;
    Point3f point;
    for (int i = 0; i < points.size(); i++)
    {
        if (i < points.size() - 1)
        {
            relativeEnd = points[i + 1] - points[i];
        }
        else
        {
            relativeEnd = points[0] - points[i];
        }

        relativePos = positionAgent - points[i];

        relativeEndScal = relativeEnd;
        relativeEndScal.normalize();

        relativePosScal = relativePos * (1.0F / relativeEnd.length());

        dotProduct = relativeEndScal.dot(relativePosScal);

        if (dotProduct < 0.0)
            nearestPoint.push_back(points[i]);
        else if (dotProduct > 1.0)
        {
            if (i < points.size() - 1)
            {
                nearestPoint.push_back(points[i + 1]);
            }
            else
            {
                nearestPoint.push_back(points[0]);
            }
        }
        else
            nearestPoint.push_back((relativeEnd * dotProduct) + points[i]);
    }

    nearestPoint.push_back(position);

    point = nearestPoint[0];

    for (int i = 0; i < nearestPoint.size(); i++)
    {
        if (positionAgent.distance(nearestPoint[i]) < positionAgent.distance(point))
        {
            point = nearestPoint[i];
        }
    }

    return point;
}

bool AGV::isNearPedes(vector<Point3f> positionList)
{
    for (Point3f p : positionList)
    {
        if (getNearestPoint(p).distance(p) < thresholdDisToPedes)
        {
            return true;
        }
    }
    return false;
}

// compare the position of the other AGV with this AGV if the other AGV is in front of this AGV
bool isInFront(Point3f position, Point3f positionOther, int generalDirection)
{
    if (generalDirection == 0)
    {
        if (position.x < positionOther.x)
        {
            return true;
        }
    }
    else if (generalDirection == 1)
    {
        if (position.x > positionOther.x)
        {
            return true;
        }
    }
    
    return false;
}

bool AGV::isNearAGV(std::vector<agvs_data> position_list_agv)
{
    // combine with isInfront function to check if the AGV is in front of this AGV
    for (agvs_data agv : position_list_agv)
    {
        if (getNearestPoint(agv.position).distance(agv.position) < thresholdDisToPedes*2 && isInFront(position, agv.position, generalDirection) && id != agv.id && !agv.reachDestination)
        {
            return true;
        }
    }
}

// void AGV::move(float stepTime, vector<Point3f> position_list)
// {
//     Vector3f velocityDiff, desiredVelocity, e_ij;

//     e_ij = getPath() - position;
//     e_ij.normalize();

//     desiredVelocity = e_ij * desiredSpeed;
//     velocityDiff = e_ij * acceleration * stepTime;

//     if (isNearPedes(position_list))
//     {
//         if (!isCollision)
//         {
//             numOfCollision++;
//         }
//         isCollision = true;
//         if (abs(velocity.x) >= abs(desiredVelocity.x) &&
//             abs(velocity.y) >= abs(desiredVelocity.y))
//         {
//             position = position + velocity * stepTime;
//             velocity = velocity - velocityDiff;
//         }
//         else
//         {
//             velocity.set(0, 0, 0);
//         }
//     }
//     else
//     {
//         isCollision = false;
//         if (velocity.length() < desiredVelocity.length())
//         {
//             position = position + velocity * stepTime;
//             velocity = velocity + velocityDiff;
//         }
//         else
//         {
//             position = position + desiredVelocity * stepTime;
//             velocity = desiredVelocity;
//         }
//     }
// }

void AGV::move(float stepTime, vector<Point3f> position_list, std::vector<agvs_data> position_list_agv)
{
    Vector3f desiredVelocity, e_ij, velocityDiff;

    e_ij = getPath() - position;
    e_ij.normalize();

    desiredVelocity = e_ij * desiredSpeed;
    velocityDiff = e_ij * acceleration * stepTime;

    if (isNearPedes(position_list) || isNearAGV(position_list_agv))
    //if (isNearPedes(position_list))
    {
        if (isNearAGV(position_list_agv))
        {
            cout << "AGV " << id << " is near AGV" << endl;
        }
        if (!isCollision)
        {
            numOfCollision++;
        }
        isCollision = true;
        if (abs(velocity.x) >= abs(desiredVelocity.x) &&
            abs(velocity.y) >= abs(desiredVelocity.y))
        {
            position = position + velocity * stepTime;
            velocity = velocity - velocityDiff;
        }
        else
        {
            velocity.set(0, 0, 0);
        }
    
    }
    else
    {
        isCollision = false;
        if ((velocity.length() < desiredVelocity.length()) && numOfCollision != 0)
        {
            position = position + velocity * stepTime;
            velocity = velocity + velocityDiff;
        }
        else
        {
            position = position + desiredVelocity * stepTime;
            velocity = desiredVelocity;
        }
    }

}

// void AGV::move(float stepTime, vector<Point3f> position_list)
// {
//     Vector3f desiredVelocity, e_ij;

//     e_ij = getPath() - position;
//     e_ij.normalize();

//     desiredVelocity = e_ij * desiredSpeed;

//     if (isNearPedes(position_list))
//     {
//         if (!isCollision)
//         {
//             numOfCollision++;
//         }
//         isCollision = true;
//         velocity.set(0, 0, 0);
//     }
//     else
//     {
//         isCollision = false;
//         velocity = desiredVelocity;
//         position = position + velocity * stepTime;
//     }

    
// }