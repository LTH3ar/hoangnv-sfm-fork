#if defined(__linux__)
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <random>
#include <lib/nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "model/SocialForce.h"
#include "constant/Constant.h"
#include "renderer/Renderer.h"
#include "src/global/Global.h"

using namespace std;
using namespace Constant;
using namespace Renderer;
using namespace GlobalConfig;

using json = nlohmann::json;

// Global Variables
GLsizei winWidth = 1080; // Window width (16:10 ratio)
GLsizei winHeight = 660; // Window height (16:10 ratio)
SocialForce *socialForce;
float fps = 0; // Frames per second
int currTime = 0;
int startTime = 0;
float predictedTime = 0;
bool animate = false; // Animate scene flag
float speedConsiderAsStop = 0.2;
json timeline_queue;

json inputData;
json previousEventData;
int eventType = 0;
int timeline_pointer = 0;
std::vector<json> Simulator_State_Stream;
float timeRatio = 1.0f;
int subtitutionTime = 0;
int runMode = 1;
int graphicsMode = 1;
int jsonOutput = 1;
int numAGVinit = 0;
int numAGVCurr = 0;
float ajustedTime = 0;



// these value will be used as the input from argv
std::vector<int> new_AGVs_Direction; // direction of the new AGV
// junction ID
std::string AGV_on_Hallway_ID = "N/A";
string state_filename = "N/A";

std::map<std::string, std::vector<float>> mapData;
std::vector<json> juncDataList;
std::vector<float> juncData;
std::string juncName;
int juncIndex = 0;
float walkwayWidth;

int classificationType = 0;

std::vector<int> numOfPeople;
float minSpeed = -1;
float maxSpeed = -1;
int threshold = 0;

// Function Prototypes
void init();
json getEventData(int timeline_pointer);
void createWalls();

void test_createAGV(int spawn_count, int direction, int agvID, float gap);
void test_createAGVs(int numberOfAGVs, vector<int> direction);
void test_createAGVs_with_previousData(vector<int> direction, json agvs_data);

void createAgents();
void createAgentAlt(json agents);
void event_handler(int eventType);
void setAgentsFlow(Agent *agent, float desiredSpeed, float maxSpeed, float minSpeed, int caseJump);
void setAgentsFlowAlt(Agent *agent, json agent_data);

void display();

void reshape(int width, int height);

void normalKey(unsigned char key, int xMousePos, int yMousePos);

void update();

int main(int argc, char **argv)
{
    // inputData = Utility::readInputData("data/input.json");
    // mapData = Utility::readMapData("data/map.txt");
    inputData = Utility::readInputData(argv[1]);
    mapData = Utility::readMapData(argv[2]);
    // remeber to convert argv[5] to string
    AGV_on_Hallway_ID = (string)inputData["hallwayID"]["value"];
    cout << "AGV on Hallway ID: " << AGV_on_Hallway_ID << endl;
    //convert argv[3] to int
    eventType = atoi(argv[3]);

    timeline_pointer = inputData["timeline_pointer"]["value"];
    cout << "Timeline pointer: " << timeline_pointer << endl;

    // use hallway id to set the file name (char *filename)
    state_filename = "data/tmp/" + AGV_on_Hallway_ID + ".json";

    //check if the file exist
    std::ifstream file(state_filename);
    if (file)
    {
        // if the file exist, read the file
        timeline_queue = Utility::readInputData(state_filename.c_str());
    }

    GlobalConfig::loadConfig();
    timeRatio = GlobalConfig::getTimeRatio();
    runMode = GlobalConfig::getRunMode();
    graphicsMode = GlobalConfig::getGraphicsMode();
    jsonOutput = GlobalConfig::getJsonOutput();
    
    juncDataList = Utility::convertMapData(mapData);
    float hallwayLength = juncDataList[juncIndex].items().begin().value();
    cout << "Hallway length: " << hallwayLength << endl;

    // cout << "Hallway length: " << hallwayLength << endl;
    // cout << "desired speed: " << inputData["agvDesiredSpeed"]["value"] << endl;

    walkwayWidth = (float)inputData["hallwayWidth"]["value"];
    float length1Side = (hallwayLength) / 2;
    juncData = {length1Side, length1Side};
    
    // Calculate predicted time: input(hallway length+9, desired speed, acceleration), output(predicted completion time)
    predictedTime = Utility::calculatePredictedTime(hallwayLength+0.8, inputData["agvDesiredSpeed"]["value"], inputData["acceleration"]["value"], timeRatio);

    cout << "Predicted time: " << predictedTime << endl;

    float deviationParam = randomFloat(1 - (float)inputData["experimentalDeviation"]["value"] / 100, 1 + (float)inputData["experimentalDeviation"]["value"] / 100);
    // Threshold people stopping at the corridor
    threshold = int(inputData["numOfAgents"]["value"]) * deviationParam * (float)(inputData["stopAtHallway"]["value"]) / 100;

    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA |
                        GLUT_DEPTH);         // Set display mode  Default mode used
    glutInitWindowSize(winWidth, winHeight); // Set window width and height
    glutInitWindowPosition(90, 90);          // Set window position
    glutCreateWindow(
        "Crowd Simulation using Social Force"); // Set window title and create
    // display window

    animate = true;
    startTime = currTime;
    if (graphicsMode == 0)
    {
        glutHideWindow();
    }

    init();                   // Initialization
    glutDisplayFunc(display); // Send graphics to display window
    glutReshapeFunc(reshape); // Maintain aspect ratio when window first created,
    // resized and moved

    glutKeyboardFunc(normalKey);
    glutIdleFunc(update); // Continuously execute 'update()'
    glutMainLoop();       // Enter GLUT's main loop

    return 0;
}

void init()
{
    // General Light Intensity
    GLfloat gnrlAmbient[] = {
        0.8F, 0.8F, 0.8F,
        1.0}; // Ambient (R, G, B, A) light intensity of entire scene

    // Object Light Intensity
    GLfloat lghtDiffuse[] = {0.7F, 0.7F, 0.7F,
                             1.0}; // Diffuse (R, G, B, A) light intensity

    // Light Position
    GLfloat lghtPosition[] = {4.0, -4.0, 4.0, 0.0};

    glClearColor(1.0, 1.0, 1.0,
                 0.0);       // Set color used when color buffer cleared
    glShadeModel(GL_SMOOTH); // Set shading option

    // General Lighting
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gnrlAmbient);

    // Object Lighting
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lghtDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lghtPosition);

    glEnable(GL_DEPTH_TEST); // Enable hidden surface removal
    glEnable(GL_NORMALIZE);  // Normalize normal vector
    glEnable(GL_LIGHTING);   // Prepare OpenGL to perform lighting calculations
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHT0);

    glCullFace(GL_BACK);    // Specify face to be culled
    glEnable(GL_CULL_FACE); // Enable culling of specified face

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);

    //srand(1604010629); // Seed to generate random numbers
    //srand(1);
    // socialForce = new SocialForce;
    // createWalls();
    // createAgents();
    // createAGVs();

    event_handler(eventType);
    //cout << "Subtitution time: " << subtitutionTime << endl;

    // // calculate the travel length of an AGV using Point3f of getPosition() and getDestination()
    // float travelLength = socialForce->getAGVs()[0]->getPosition().distance(socialForce->getAGVs()[0]->getDestination());
    // cout << "Travel length: " << travelLength << endl;

    // add to state list
    ajustedTime += (float)timeline_pointer;
    json current_State = Utility::SaveState(socialForce->getAGVs(), socialForce->getCrowd(), ajustedTime*1000.0f, timeline_pointer);
    Simulator_State_Stream.push_back(current_State);
}

void event_handler(int eventType)
{
    numAGVinit = inputData["agvIDs"]["value"].size();
    
    new_AGVs_Direction = (vector<int>)inputData["agvDirections"]["value"];
    //cout << "AGV Direction: " << new_AGVs_Direction[0] << endl;
    if (eventType == 0)
    {
        socialForce = new SocialForce;
        createWalls();
        createAgents();
        //createAGVs();
        test_createAGVs(numAGVinit, new_AGVs_Direction);
        
    }
    else
    {
        // get the previous event data
        previousEventData = getEventData(timeline_pointer);
        socialForce = new SocialForce;
        createWalls();
        createAgentAlt(previousEventData["agents"]);
        //createAGVsAlt(previousEventData["agvs"]);
        numAGVinit = numAGVinit + previousEventData["agvs"].size();
        test_createAGVs_with_previousData(new_AGVs_Direction, previousEventData["agvs"]);
    }
    numAGVCurr = socialForce->getAGVs().size();
    cout << "Number of AGVs: " << numAGVCurr << endl;
}

json getEventData(int timeline_pointer)
{
    json eventData;
    for (json event : timeline_queue["timeline"])
    {
        // both ['timeline']['event_time'] and eventTime must be converted to interger after 
        if ((int)(event["event_time"].get<int>()) == timeline_pointer)
        {
            eventData = event;
            break;
        }
    }
    return eventData;
}

void createWalls()
{
    Wall *wall;
    //cout << "Junction size: " << juncData.size() << endl;

    vector<float> coors = Utility::getWallCoordinates(walkwayWidth, juncData);

    if (juncData.size() )
    {
        // Upper Wall
        wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
        socialForce->addWall(wall);
        // Lower Wall
        wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
        socialForce->addWall(wall);
    }
    
}

void setAgentsFlow(Agent *agent, float desiredSpeed, float maxSpeed, float minSpeed, int caseJump)
{
    // if (socialForce->getCrowdSize() < threshold)
    // {
    //     agent->setStopAtCorridor(true);
    // }

    int codeSrc = 0;
    int codeDes = 0;

    int juncType = juncData.size();
    
    if (juncType == 2)
    {
        if (caseJump < 3)
        {
            codeSrc = 0;
        }
        else
        {
            codeSrc = 1;
        }
    }

    vector<float> position = Utility::getPedesSource(
        codeSrc,
        (float)inputData["totalCrowdLength"]["value"],
        (float)inputData["headCrowdLength"]["value"],
        (float)inputData["crowdWidth"]["value"],
        walkwayWidth,
        juncData);
    vector<float> desList;
    if (juncType == 2)
    {
        desList = Utility::getPedesDestination(codeSrc, caseJump % 3, walkwayWidth, juncData, agent->getStopAtCorridor());
    }

    agent->setPosition(position[0], position[1]);
    
    agent->setPath(desList[0], desList[1], desList[2]);
    agent->setDestination(desList[0], desList[1]);
    agent->setDesiredSpeed(desiredSpeed);
    std::vector<float> color = getPedesColor(maxSpeed, minSpeed, agent->getDesiredSpeed(), classificationType);
    agent->setColor(color[0], color[1], color[2]);
    //cout << "Color: " << color[0] << " - " << color[1] << " - " << color[2] << endl;
    socialForce->addAgent(agent);
}

// alternate function to set agents from the previous event
void setAgentsFlowAlt(Agent *agent, json agent_data)
{
    agent->setPosition(agent_data["position"][0], agent_data["position"][1]);
    agent->setPath(agent_data["path"][0]["position"][0], agent_data["path"][0]["position"][1], agent_data["path"][0]["radius"]);
    agent->setDestination(agent_data["destination"][0], agent_data["destination"][1]);
    agent->setDesiredSpeed(agent_data["desiredSpeed"]);
    agent->setColor(agent_data["color"][0], agent_data["color"][1], agent_data["color"][2]);
    agent->setVelocity(agent_data["velocity"][0], agent_data["velocity"][1], agent_data["velocity"][2]);
    socialForce->addAgent(agent);
}

void createAgents()
{
    Agent *agent;

    float deviationParam = randomFloat(1 - (float)inputData["experimentalDeviation"]["value"] / 100, 1 + (float)inputData["experimentalDeviation"]["value"] / 100);
    // cout << "Deviation: "<< deviationParam <<" - Num agents: "<< int(int(inputData["numOfAgents"]["value"]) * deviationParam) << endl;
    numOfPeople = Utility::getNumPedesInFlow(juncData.size(), int(int(inputData["numOfAgents"]["value"]) * deviationParam));
    vector<double> velocityList = Utility::getPedesVelocity(classificationType, inputData, deviationParam);
    if (classificationType == 0)
    {
        minSpeed = 0.52;
        maxSpeed = 2.28;
    }
    else
    {
        minSpeed = velocityList[0];
        maxSpeed = velocityList[velocityList.size() - 1];
    }

    auto rng = std::default_random_engine{};
    std::shuffle(velocityList.begin(), velocityList.end(), rng);

    int pedesCount = 0;

    // test

    // for (int temp = 0; temp < 3; temp++)
    // {
    //     agent = new Agent;
    //     // setAgentsFlow(agent, 1, maxSpeed, minSpeed, Point3f(randomFloat(-3.0, -2.0), randomFloat(9.0, 10.0), 0.0), Point3f(randomFloat(-3.2, -2.8), randomFloat(-2.2, -1.8), 0.0));
    //     agent->setPosition(randomFloat(-3.0, -2.0), randomFloat(9.0, 10.0));
    //     // // float x = randomFloat(-13.3F, -6.0);
    //     // // float y = randomFloat(-2.0, 2.0);
    //     float x = randomFloat(-3.2, -2.8);
    //     float y = randomFloat(-2.2, -1.8);
    //     agent->setPath(x, y, 2);
    //     agent->setDestination(x, y);
    //     // // agent->setPath(randomFloat(22.0, 25.0), randomFloat(-3.0, -2.0), 1.0);
    //     agent->setDesiredSpeed(1);
    //     agent->setStopAtCorridor(true);
    //     std::vector<float> color = Utility::getPedesColor(maxSpeed, minSpeed, agent->getDesiredSpeed(), classificationType);
    //     agent->setColor(color[0], color[1], color[2]);
    //     socialForce->addAgent(agent);
    // }

    // test

    if (juncData.size() == 2)
    {
        for (int idx = 0; idx < 6; idx++)
        {
            for (int temp = 0; temp < numOfPeople[idx]; temp++)
            {
                agent = new Agent;
                setAgentsFlow(agent, velocityList[pedesCount], maxSpeed, minSpeed, idx);
                pedesCount = pedesCount + 1;
            }
        }
    }
    
}

void createAgentAlt(json agents)
{
    Agent *agent;
    // get the number of agents from previous event
    int numOfPeople = agents.size();

    for (int i = 0; i < numOfPeople; i++)
    {
        agent = new Agent;
        setAgentsFlowAlt(agent, agents[i]);
    }

}

//redesign the function to create AGVs
//create a single AGV
void test_createAGV(int spawn_count, int direction, int agvID, float gap)
{
    AGV *agv = new AGV();
    float agv_length = 0.75;
    //float gap = 0.3;

    float hallwayLength = juncDataList[0].items().begin().value();
    float length1Side = hallwayLength / 2;
    vector<float> juncDataTemp = {length1Side, length1Side};
    vector<Point3f> route = Utility::getRouteAGV(0, 1, walkwayWidth, juncDataTemp);
    
    agv->setDirection(direction, 1);
    if (direction == 0) // Left to right
    {
        agv->setPosition(route[0].x - (float)(spawn_count*(agv_length+gap)), route[0].y);
        agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
    }
    else // Right to left
    {
        agv->setPosition(-route[0].x + (float)(spawn_count*(agv_length+gap)), -route[0].y);
        agv->setDestination(-route[route.size() - 1].x, -route[route.size() - 1].y);
    }
    
    agv->setDesiredSpeed((float)inputData["agvDesiredSpeed"]["value"]);
    agv->setAcceleration(inputData["acceleration"]["value"]);
    agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
    
    for (int i = 1; i < route.size(); i++)
    {
        if (direction == 0)
            agv->setPath(route[i].x, route[i].y, 1.0);
        else
            agv->setPath(-route[i].x, -route[i].y, 1.0);
    }

    agv->setAgvIdx(agvID);
    agv->setGeneralDirection(direction);

    socialForce->addAGV(agv);
}

// Function to create multiple AGVs
void test_createAGVs(int numberOfAGVs, vector<int> direction)
{
    vector<int> left2right;
    vector<int> right2left;
    for (int i = 0; i < numberOfAGVs; i++)
    {
        if (direction[i] == 0)
        {
            left2right.push_back(i);
        }
        else
        {
            right2left.push_back(i);
        }
    }
    for (int i = 0; i < left2right.size(); i++)
    {
        test_createAGV(i, 0, inputData["agvIDs"]["value"][left2right[i]], 0.3);
    }
    for (int i = 0; i < right2left.size(); i++)
    {
        test_createAGV(i, 1, inputData["agvIDs"]["value"][right2left[i]], 0.3);
    }
}

void test_createAGVs_with_previousData(vector<int> direction, json agvs_data)
{
    // init all the AGVs from the previous event
    for (int i = 0; i < agvs_data.size(); i++)
    {
        test_createAGV(i, agvs_data[i]["generalDirection"], agvs_data[i]["id"], 0.3);
    }

    for (AGV *agv : socialForce->getAGVs())
    {
        for (json::iterator it = agvs_data.begin(); it != agvs_data.end(); ++it)
        {
            if (agv->getId() == it.value()["id"])
            {
                agv->setThresholdDisToPedes((float)it.value()["thresholdDisToPedes"]);
                agv->setAcceleration((float)it.value()["acceleration"]);
                agv->setDirection(it.value()["direction"][0], it.value()["direction"][1]);
                agv->setPosition((float)it.value()["position"][0], (float)it.value()["position"][1]);
                agv->setVelocity((float)it.value()["velocity"][0], (float)it.value()["velocity"][1], (float)it.value()["velocity"][2]);
            }
        }
    }

    // init new AGVs
    vector<int> left2right;
    vector<int> right2left;
    for (int i = 0; i < direction.size(); i++)
    {
        if (direction[i] == 0)
        {
            left2right.push_back(i);
        }
        else
        {
            right2left.push_back(i);
        }
    }
    for (int i = 0; i < left2right.size(); i++)
    {
        test_createAGV(i, 0, inputData["agvIDs"]["value"][left2right[i]], 0.3);
    }
    for (int i = 0; i < right2left.size(); i++)
    {
        test_createAGV(i, 1, inputData["agvIDs"]["value"][right2left[i]], 0.3);
    }

}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffer
    glLoadIdentity();             // Initialize modelview matrix to identity matrix

    // Camera
    gluLookAt(0.0, 0.0, 18.0, // Position
              0.0, 0.0, 0.0,  // Look-at point
              0.0, 1.0, 0.0); // Up-vector

    glPushMatrix();
    glScalef(1.0, 1.0, 1.0);

    drawAgents(socialForce);
    drawAGVs(socialForce, juncData, runMode);
    drawWalls(socialForce);
    glPopMatrix();

    showInformation(socialForce, fps, animate, currTime, startTime, classificationType, winWidth, winHeight);

    glutSwapBuffers();
}

void reshape(int width, int height)
{
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialize projection matrix to identity matrix
    gluPerspective(65.0, static_cast<GLfloat>(width) / height, 1.0,
                   100.0); // Create matrix for symmetric perspective-view frustum

    glMatrixMode(GL_MODELVIEW);

    winWidth = width;
    winHeight = height;
}

void normalKey(unsigned char key, int xMousePos, int yMousePos)
{
    switch (key)
    {
    case 'a': // Animate or inanimate scene
        animate = (!animate) ? true : false;
        startTime = currTime;
        break;

    case 27: // ASCII character for Esc key
        delete socialForce;
        socialForce = 0;

        exit(0); // Terminate program
        break;
    }
}

// Update function to update the scene(to be called continuously to move the agents and AGVs)
void update()
{
    static int prevTime;
    currTime = glutGet(GLUT_ELAPSED_TIME); // Get time in milliseconds since 'glutInit()' called
    int frameTime = currTime - prevTime;
    float stepTime = frameTime / 1000.0f * timeRatio; // this one in seconds

    // ajustedTime in milliseconds
    ajustedTime += stepTime*1000.0f;

    prevTime = currTime;

    int count_agents = 0, count_agvs = 0;

    // History of the simulation
    // json current_State = Utility::SaveState(socialForce->getAGVs(), socialForce->getCrowd(), (currTime + (timeline_pointer*1000))*(1000/timeRatio));
    // Simulator_State_Stream.push_back(current_State);

    for (Agent *agent : socialForce->getCrowd())
    {
        Point3f src = agent->getPosition();
        Point3f des = agent->getDestination();
        
        if (Utility::isPositionErr(src, walkwayWidth, juncData.size(), socialForce->getAGVs()))
        {
            socialForce->removeAgent(agent->getId());
            continue;
        }

        if (agent->getVelocity().length() < LOWER_SPEED_LIMIT + 0.2 &&
            agent->getMinDistanceToWalls(socialForce->getWalls(), src, agent->getRadius()) < 0.2 &&
            (agent->interDes).size() == 0)
        {
            Point3f intermediateDes = Utility::getIntermediateDes(src, walkwayWidth, walkwayWidth);

            (agent->interDes).push_back(intermediateDes);
            agent->setPath(intermediateDes.x, intermediateDes.y, 1.0);
            agent->setPath(des.x, des.y, 1.0);
        }

        if ((agent->interDes).size() > 0)
        {
            float distanceToInterDes = src.distance((agent->interDes).front());
            if (distanceToInterDes <= 1)
            {
                (agent->interDes).clear();
            }
        }

        float distanceToTarget = src.distance(des);
        if (distanceToTarget <= 1 || isnan(distanceToTarget))
        {
            agent->setIsMoving(false);
            if (!agent->getStopAtCorridor())
            {
                socialForce->removeAgent(agent->getId());
            }
            count_agents = count_agents + 1;
        }
        //cout << "AgentID: " << agent->getId() << " - Source: " << src << " - Destination: " << des << "Time: " << run_time << " Current_Speed: " << agent->getVelocity().length() << endl;
    }

    // std::vector<AGV *> agvs = socialForce->getAGVs();
    for (AGV *agv : socialForce->getAGVs())
    {
        if (agv->getCollisionStartTime() == 0 && agv->getVelocity().length() < speedConsiderAsStop && agv->getIsMoving())
        {
            agv->setCollisionStartTime(glutGet(GLUT_ELAPSED_TIME));
            // cout << "- Start collision: " << convertTime(agv->getCollisionStartTime()) << endl;
        }

        if (agv->getCollisionStartTime() != 0 && agv->getVelocity().length() > speedConsiderAsStop && agv->getIsMoving())
        {
            agv->setTotalStopTime(agv->getTotalStopTime() + glutGet(GLUT_ELAPSED_TIME) - agv->getCollisionStartTime());
            // cout << "- Stop collision: " << convertTime(glutGet(GLUT_ELAPSED_TIME)) << endl;
            // cout << "=> Total collision: " << convertTime(agv->getTotalStopTime()) << endl;
            agv->setCollisionStartTime(0);
        }

        Point3f src = agv->getPosition();
        //std::cout << "Source: " << src << endl;
        Point3f des = agv->getDestination();
        //std::cout << "Destination: " << des << endl;
        

        float distance = src.distance(des);
        if (distance <= 1 || isnan(distance))
        {
            agv->setReachDestination(true);
            if (agv->getIsMoving())
            {
                agv->setTravelingTime(glutGet(GLUT_ELAPSED_TIME) - agv->getTravelingTime());
                std::cout << "Traveling time: " << convertTime(agv->getTravelingTime()*timeRatio) << endl;
                agv->setIsMoving(false);

                int numAGVCompleted = getNumAGVCompleted(socialForce->getAGVs());

                int marker = numAGVCurr;
                if (numAGVCompleted % numAGVCurr == 0)
                {
                    socialForce->removeCrowd();
                    createAgents();
                }

                if (numAGVCompleted > 0 && numAGVCompleted % marker == 0)
                {
                    juncIndex = juncIndex + 1;
                    if (juncIndex == juncDataList.size())
                    {
                        juncIndex = 0;
                    }
                    socialForce->removeWalls();
                    float hallwayLength = juncDataList[juncIndex].items().begin().value();
                    if (numAGVCompleted + 1 <= socialForce->getAGVs().size())
                    {
                        cout << "*****=> " << juncDataList[juncIndex].items().begin().key() << ": " << hallwayLength << endl;
                    }
                    float length1Side = (hallwayLength) / 2;
                    juncData = {length1Side, length1Side};
                    createWalls();
                    // cout << agv->getId() << " - Remove and re-create agent" << endl;
                }
            }
            count_agvs = count_agvs + 1;
        }

       //cout << "AGV ID: " << agv->getId() << " - Source: " << src << " - Destination: " << des << "Time: " << run_time << " Current_Speed: " << agv->getVelocity().length() << " Reach Destination: " << agv->getReachDestination() << endl;
    }
    // History of the simulation
    json current_State = Utility::SaveState(socialForce->getAGVs(), socialForce->getCrowd(), ajustedTime, timeline_pointer);
    Simulator_State_Stream.push_back(current_State);

    if (count_agvs == socialForce->getAGVs().size())
    {
        int totalRunningTime = currTime - startTime;
        
        Utility::writeResult(
            "data/end.txt", juncName, graphicsMode, socialForce->getAGVs(),
            juncDataList,
            runMode,
            totalRunningTime,
            AGV_on_Hallway_ID,
            timeRatio,
            timeline_pointer,
            eventType,
            inputData["agvIDs"]["value"]);

        // get the list of agv IDs
        std::vector<int> endAgvIDs;
        for (AGV *agv : socialForce->getAGVs())
        {
            endAgvIDs.push_back(agv->getId());
        }

        Utility::timeline_writer("data/timeline/timeline.json", AGV_on_Hallway_ID.c_str(), timeline_pointer, ajustedTime, endAgvIDs);

        //std::cout << "Maximum speed: " << maxSpeed << " - Minimum speed: " << minSpeed << endl;
        std::cout << "Finish in: " << totalRunningTime << endl;
        delete socialForce;
        socialForce = 0;

        //input : const char *fileName, std::vector<json> stateList
        Utility::writeState(state_filename.c_str(), Simulator_State_Stream, timeline_pointer);
        
        exit(0); // Terminate program
    }

    if (animate)
    {
        // socialForce->moveCrowd(static_cast<float>(frameTime) / 1000); // Perform calculations and move agents
        // socialForce->moveAGVs(static_cast<float>(frameTime) / 1000);
        socialForce->moveCrowd(stepTime); // Perform calculations and move agents
        socialForce->moveAGVs(stepTime);
    }
    computeFPS(&fps);
    glutPostRedisplay();
    glutIdleFunc(update); // Continuously execute 'update()'
}
// tích hợp hành vi người đi bộ vào phần mêm mô phỏng thuật toán định tuyến AGV