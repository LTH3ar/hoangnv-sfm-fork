#if defined(__linux__)
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <random>
#include <lib/nlohmann/json.hpp>

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
int predictedTime = 0;
bool animate = false; // Animate scene flag
float speedConsiderAsStop = 0.2;

json inputData;
json previousEventData;
int eventTime = 0;
int timeRatio = 1;
int runMode = 1;
int graphicsMode = 1;
int jsonOutput = 1;

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

void createWalls();

void createAgents();
void createAgentsPastEvent(int time, std::vector<Agent *> agents);

void createAGVs();

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
    eventTime = (int)argv[3]; //seconds

    GlobalConfig::loadConfig();
    timeRatio = GlobalConfig::getTimeRatio();
    runMode = GlobalConfig::getRunMode();
    graphicsMode = GlobalConfig::getGraphicsMode();
    jsonOutput = GlobalConfig::getJsonOutput();

    std::string input1;
    
    juncDataList = Utility::convertMapData(mapData);
    float hallwayLength = juncDataList[juncIndex].items().begin().value();

    cout << "Hallway length: " << hallwayLength << endl;
    cout << "desired speed: " << inputData["agvDesiredSpeed"]["value"] << endl;

    walkwayWidth = (float)inputData["hallwayWidth"]["value"];
    float length1Side = (hallwayLength) / 2;
    juncData = {length1Side, length1Side};
    
    // Calculate predicted time: input(hallway length+9, desired speed, acceleration), output(predicted completion time)
    predictedTime = Utility::calculatePredictedTime(hallwayLength + 9, inputData["agvDesiredSpeed"]["value"], inputData["acceleration"]["value"], timeRatio);

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

    srand(1604010629); // Seed to generate random numbers

    socialForce = new SocialForce;
    createWalls();
    createAgents();
    createAGVs();
}

void createWalls()
{
    Wall *wall;

    vector<float> coors = Utility::getWallCoordinates(walkwayWidth, juncData);

    if (juncData.size() == 2)
    {
        // Upper Wall
        wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
        socialForce->addWall(wall);
        // Lower Wall
        wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
        socialForce->addWall(wall);
    }
    else
    {
        // Upper Wall
        if (juncData.size() == 4)
        {
            wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
            socialForce->addWall(wall);

            wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
            socialForce->addWall(wall);
        }
        else if (juncData.size() == 3)
        {
            wall = new Wall(coors[0], coors[1], coors[6], coors[7]);
            socialForce->addWall(wall);
        }

        // Lower Wall
        wall = new Wall(coors[8], coors[9], coors[10], coors[11]);
        socialForce->addWall(wall);

        wall = new Wall(coors[12], coors[13], coors[14], coors[15]);
        socialForce->addWall(wall);

        // Left Wall
        if (juncData.size() == 4)
        {
            wall = new Wall(coors[16], coors[17], coors[18], coors[19]);
            socialForce->addWall(wall);
        }

        wall = new Wall(coors[20], coors[21], coors[22], coors[23]);
        socialForce->addWall(wall);

        // Right Wall
        if (juncData.size() == 4)
        {
            wall = new Wall(coors[24], coors[25], coors[26], coors[27]);
            socialForce->addWall(wall);
        }

        wall = new Wall(coors[28], coors[29], coors[30], coors[31]);
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

    if (juncType == 4)
    {
        if (caseJump < 3)
        {
            codeSrc = 0; // Go from Left to Right
        }
        else if (caseJump < 6)
        {
            codeSrc = 1; // Go from Right to Left
        }
        else if (caseJump < 9)
        {
            codeSrc = 2; // Go from Top to Bottom
        }
        else
        {
            codeSrc = 3; // Go from Bottom to Top
        }
    }
    else if (juncType == 3)
    {
        if (caseJump < 6)
        {
            codeSrc = 0;
            if (caseJump % 2 == 0)
            {
                codeDes = 0;
            }
            else
            {
                codeDes = 2;
            }
        }
        else if (caseJump < 12)
        {
            codeSrc = 1;
            if (caseJump % 2 == 0)
            {
                codeDes = 1;
            }
            else
            {
                codeDes = 2;
            }
        }
        else if (caseJump < 18)
        {
            codeSrc = 3;
            if (caseJump % 2 == 0)
            {
                codeDes = 0;
            }
            else
            {
                codeDes = 1;
            }
        }
    }
    else if (juncType == 2)
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
    if (juncType == 4 || juncType == 2)
    {
        desList = Utility::getPedesDestination(codeSrc, caseJump % 3, walkwayWidth, juncData, agent->getStopAtCorridor());
    }
    else if (juncType == 3)
    {
        desList = Utility::getPedesDestination(codeDes, caseJump % 3, walkwayWidth, juncData, agent->getStopAtCorridor());
    }

    agent->setPosition(position[0], position[1]);
    if (juncType == 3 && codeSrc != codeDes)
    {
        agent->setPath(randomFloat(-walkwayWidth / 2, walkwayWidth / 2), randomFloat(-walkwayWidth / 2, walkwayWidth / 2), 2.0);
    }
    agent->setPath(desList[0], desList[1], desList[2]);
    agent->setDestination(desList[0], desList[1]);
    agent->setDesiredSpeed(desiredSpeed);
    std::vector<float> color = getPedesColor(maxSpeed, minSpeed, agent->getDesiredSpeed(), classificationType);
    agent->setColor(color[0], color[1], color[2]);
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
    else if (juncData.size() == 3)
    {
        for (int idx = 0; idx < 18; idx++)
        {
            for (int temp = 0; temp < numOfPeople[idx]; temp++)
            {
                agent = new Agent;
                setAgentsFlow(agent, velocityList[pedesCount], maxSpeed, minSpeed, idx);
                pedesCount = pedesCount + 1;
            }
        }
    }
    else if (juncData.size() == 4)
    {
        for (int idx = 0; idx < 12; idx++)
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

// alternate function to create agents from the previous event


void createAGVs()
{
    AGV *agv = NULL;
    vector<int> array;

    // test
    // agv = new AGV();
    // vector<Point3f> route = Utility::getRouteAGV(juncData.size(), 0, 2, walkwayWidth, juncData);
    // agv->setDirection(0, 2);
    // agv->setPosition(route[0].x, route[0].y);

    // for (Agent *agent : socialForce->getCrowd())
    // {
    //     if (agent->getPosition().distance(agv->getPosition()) < 0.5F)
    //     {
    //         do
    //         {
    //             agent->setPosition(agent->getPosition().x - 0.1F, agent->getPosition().y - 0.1F);
    //         } while (agent->getPosition().distance(agv->getPosition()) < 0.5F);
    //     }
    // }

    // agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
    // agv->setDesiredSpeed(0.6F);
    // agv->setAcceleration(inputData[9]);
    // agv->setDistance((float)inputData[10]);
    // for (int i = 1; i < route.size(); i++)
    // {
    //     agv->setPath(route[i].x, route[i].y, 1.0);
    //     std::cout << route[i] << endl;
    // }
    // socialForce->addAGV(agv);

    // test
    
    int numOfHallway = juncDataList.size();
    int numRunPerHallway = (int)inputData["noRunPerHallway"]["value"];
    int juncIndexTemp = 0;
    float hallwayLength = juncDataList[juncIndexTemp].items().begin().value();
    // cout << "*****=> " << juncDataList[juncIndex].items().begin().key() << ": " << hallwayLength << endl;
    float length1Side = (hallwayLength) / 2;
    vector<float> juncDataTemp = {length1Side, length1Side};
    int numAGVPerRun = 1;
    int agvDirection = 0; // Opposite direction
    if ((int)inputData["runConcurrently"]["value"] == 1)
    {
        numAGVPerRun = 2;
    }
    // std::cout << int(inputData["runDirection"]["value"]) << endl;
    if (int(inputData["runDirection"]["value"]) == 1)
    {
        agvDirection = 1; // Same direction
    }
    // std::cout << "Direction: " << agvDirection << endl;
    for (int i = 0; i < numOfHallway * numRunPerHallway; i++)
    {
        
        if (agvDirection == 0){
        
            for (int j = 0; j < numAGVPerRun; j++)
            {
                // std::cout << "AGV: " << j << endl;
                agv = new AGV();
                vector<Point3f> route = Utility::getRouteAGV(j, 1, walkwayWidth, juncDataTemp); // Just need the source
                agv->setDirection(j, 1);
                // std::cout << "Direction: " << j << " - 1" << endl;
                // std::cout << "Route: " << route[0] << endl;
                agv->setPosition(route[0].x, route[0].y);

                agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
                // std::cout << "Destination: " << route[route.size() - 1] << endl;

                agv->setDesiredSpeed((float)inputData["agvDesiredSpeed"]["value"]);
                // std::cout << "Desired speed: " << (float)inputData["agvDesiredSpeed"]["value"] << endl;
                
                agv->setAcceleration(inputData["acceleration"]["value"]);
                // std::cout << "Acceleration: " << inputData["acceleration"]["value"] << endl;

                agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
                // std::cout << "Threshold distance: " << (float)inputData["thresDistance"]["value"] << endl;

                for (int i = 1; i < route.size(); i++)
                {
                    agv->setPath(route[i].x, route[i].y, 1.0);
                    // std::cout << "Path: " << route[i] << endl;
                }
                socialForce->addAGV(agv);

                int marker = numRunPerHallway * (juncIndexTemp + 1) - 1;
                if ((int)inputData["runConcurrently"]["value"] == 1)
                {
                    marker = numRunPerHallway * 2 * (juncIndexTemp + 1) - 1;
                    // std::cout << "Marker: " << marker << endl;
                }
                // std::cout << "AGV ID: " << agv->getId() << endl;
                if (agv->getId() == marker)
                {
                    juncIndexTemp = juncIndexTemp + 1;
                    // std::cout << "Junction index: " << juncIndexTemp << endl;

                    if (juncIndexTemp == juncDataList.size())
                    {
                        juncIndexTemp = 0;
                        // std::cout << "Junction index: " << juncIndexTemp << endl;
                    }
                    hallwayLength = juncDataList[juncIndexTemp].items().begin().value();
                    // std::cout << "*****=> " << juncDataList[juncIndexTemp].items().begin().key() << ": " << hallwayLength << endl;
                    length1Side = (hallwayLength) / 2;
                    // std::cout << "Length 1 side: " << length1Side << endl;
                    juncDataTemp = {length1Side, length1Side};
                    // std::cout << "Junction data: " << juncDataTemp[0] << " - " << juncDataTemp[1] << endl;
                }
                // std::cout << "====================================" << endl << endl;
            }
        
        }
        
        else {

            for (int j = 0; j < numAGVPerRun; j++)
            {
                // std::cout << "AGV: " << j << endl;
                agv = new AGV();
                
                if (j == 0){
                    vector<Point3f> route = Utility::getRouteAGV(j, 1, walkwayWidth, juncDataTemp); // Just need the source
                    agv->setDirection(j, 1);
                    // std::cout << "Direction: " << j << " - 1" << endl;
                    // std::cout << "Route: " << route[0] << endl;
                    agv->setPosition(route[0].x, route[0].y);

                    agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
                    // std::cout << "Destination: " << route[route.size() - 1] << endl;
                    for (int i = 1; i < route.size(); i++)
                    {
                        agv->setPath(route[i].x, route[i].y, 1.0);
                        // std::cout << "Path: " << route[i] << endl;
                    }
                }
                else {
                    vector<Point3f> route = Utility::getRouteAGV(j, 1, walkwayWidth, juncDataTemp); // Just need the source
                    agv->setDirection(0, 1);
                    // std::cout << "Direction: " << 0 << " - 1" << endl;
                    // std::cout << "Route: " << route[0] << endl;
                    agv->setPosition(-route[0].x, route[0].y);

                    agv->setDestination(-route[route.size() - 1].x, route[route.size() - 1].y);
                    // std::cout << "Destination: " << route[route.size() - 1] << endl;
                    for (int i = 1; i < route.size(); i++)
                    {
                        agv->setPath(-route[i].x, route[i].y, 1.0);
                        // std::cout << "Path: " << route[i] << endl;
                    }
                }

                agv->setDesiredSpeed((float)inputData["agvDesiredSpeed"]["value"]);
                // std::cout << "Desired speed: " << (float)inputData["agvDesiredSpeed"]["value"] << endl;
                
                agv->setAcceleration(inputData["acceleration"]["value"]);
                // std::cout << "Acceleration: " << inputData["acceleration"]["value"] << endl;

                agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
                // std::cout << "Threshold distance: " << (float)inputData["thresDistance"]["value"] << endl;

                
                socialForce->addAGV(agv);

                int marker = numRunPerHallway * (juncIndexTemp + 1) - 1;
                if ((int)inputData["runConcurrently"]["value"] == 1)
                {
                    marker = numRunPerHallway * 2 * (juncIndexTemp + 1) - 1;
                    // std::cout << "Marker: " << marker << endl;
                }
                // std::cout << "AGV ID: " << agv->getId() << endl;
                if (agv->getId() == marker)
                {
                    juncIndexTemp = juncIndexTemp + 1;
                    // std::cout << "Junction index: " << juncIndexTemp << endl;

                    if (juncIndexTemp == juncDataList.size())
                    {
                        juncIndexTemp = 0;
                        // std::cout << "Junction index: " << juncIndexTemp << endl;
                    }
                    hallwayLength = juncDataList[juncIndexTemp].items().begin().value();
                    // std::cout << "*****=> " << juncDataList[juncIndexTemp].items().begin().key() << ": " << hallwayLength << endl;
                    length1Side = (hallwayLength) / 2;
                    // std::cout << "Length 1 side: " << length1Side << endl;
                    juncDataTemp = {length1Side, length1Side};
                    // std::cout << "Junction data: " << juncDataTemp[0] << " - " << juncDataTemp[1] << endl;
                }
                // std::cout << "====================================" << endl << endl;
            }

        }
        
    
    
    
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
    drawAGVs(socialForce, juncData, (int)inputData["runConcurrently"]["value"], runMode);
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

std::vector<json> Simulator_State_Stream;

// Update function to update the scene(to be called continuously to move the agents and AGVs)
void update()
{
    int frameTime;       // Store time in milliseconds
    static int prevTime; // Stores time in milliseconds

    currTime = glutGet(GLUT_ELAPSED_TIME); // Get time in milliseconds since 'glutInit()' called
    frameTime = currTime - prevTime;
    prevTime = currTime;

    int count_agents = 0, count_agvs = 0;

    std::vector<Agent *> agents = socialForce->getCrowd();
    string run_time = convertTime((currTime - startTime)*(1000/timeRatio));

    // History of the simulation
    json current_State = Utility::SaveState(socialForce->getAGVs(), agents, currTime*(1000/timeRatio));
    Simulator_State_Stream.push_back(current_State);

    for (Agent *agent : agents)
    {
        Point3f src = agent->getPosition();
        Point3f des = agent->getDestination();
        
        cout << "AgentID: " << agent->getId() << " - Source: " << src << " - Destination: " << des << "Time: " << run_time << " Current_Speed: " << agent->getVelocity().length() << endl;
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
    }

    std::vector<AGV *> agvs = socialForce->getAGVs();
    for (AGV *agv : agvs)
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
        cout << "AGV ID: " << agv->getId() << " - Source: " << src << " - Destination: " << des << "Time: " << run_time << " Current_Speed: " << agv->getVelocity().length() << endl;

        float distance = src.distance(des);
        if (distance <= 1 || isnan(distance))
        {
            if (agv->getIsMoving())
            {
                agv->setTravelingTime(glutGet(GLUT_ELAPSED_TIME) - agv->getTravelingTime());
                std::cout << "Traveling time: " << convertTime(agv->getTravelingTime()*(1000/timeRatio)) << endl;
                agv->setIsMoving(false);

                int numAGVCompleted = getNumAGVCompleted(socialForce->getAGVs());

                int marker = (int)inputData["noRunPerHallway"]["value"];
                if ((int)inputData["runConcurrently"]["value"] == 1)
                {
                    marker = (int)inputData["noRunPerHallway"]["value"] * 2;
                    if (numAGVCompleted % 2 == 0)
                    {
                        socialForce->removeCrowd();
                        createAgents();
                    }
                }
                else
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
                    if (numAGVCompleted + 1 <= agvs.size())
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
    }
    if (count_agvs == agvs.size())
    {
        int totalRunningTime = currTime - startTime;

        // if (currTime >= predictedTime){
        //     cout << "Time out" << endl;
        //     Utility::writeResult(
        //         "data/end.txt", juncName, graphicsMode, agvs,
        //         juncDataList,
        //         (int)inputData["runConcurrently"]["value"],
        //         runMode,
        //         (int)inputData["noRunPerHallway"]["value"],
        //         totalRunningTime,
        //         timeRatio);
        //     Utility::writeState("data/tmp/state.json", Simulator_State_Stream);
        //     exit(0); // Terminate program
        // }
        
        Utility::writeResult(
            "data/end.txt", juncName, graphicsMode, agvs,
            juncDataList,
            (int)inputData["runConcurrently"]["value"],
            runMode,
            (int)inputData["noRunPerHallway"]["value"],
            totalRunningTime,
            timeRatio);

        std::cout << "Maximum speed: " << maxSpeed << " - Minimum speed: " << minSpeed << endl;
        std::cout << "Finish in: " << Utility::convertTime(totalRunningTime) << totalRunningTime << endl;
        delete socialForce;
        socialForce = 0;

        Utility::writeState("data/tmp/state.json", Simulator_State_Stream);

        exit(0); // Terminate program
    }

    if (animate)
    {
        // socialForce->moveCrowd(static_cast<float>(frameTime) / 1000); // Perform calculations and move agents
        // socialForce->moveAGVs(static_cast<float>(frameTime) / 1000);
        socialForce->moveCrowd(static_cast<float>(frameTime) / timeRatio); // Perform calculations and move agents
        socialForce->moveAGVs(static_cast<float>(frameTime) / timeRatio);
    }
    computeFPS(&fps);
    glutPostRedisplay();
    glutIdleFunc(update); // Continuously execute 'update()'
}
// tích hợp hành vi người đi bộ vào phần mêm mô phỏng thuật toán định tuyến AGV