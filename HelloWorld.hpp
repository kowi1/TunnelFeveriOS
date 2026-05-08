#ifndef endlesstunnel_hello_hpp
#define endlesstunnel_hello_hpp

#include "btBulletDynamicsCommon.h"
#include "HelloWorld.hpp"
#include "common.hpp"
#include "game_consts.hpp"
#include <stdio.h>

class Hello{
	public:
	Hello();
    ~Hello();
    void Loop();
    void AddShapes(int r,int c,int section,int x,int y,int z);
    btDiscreteDynamicsWorld* dynamicsWorld ;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    btSequentialImpulseConstraintSolver* solver;
    btBroadphaseInterface* overlappingPairCache ;
    btDefaultCollisionConfiguration* collisionConfiguration ;
    btCollisionDispatcher* dispatcher ;
    btCollisionShape* groundShape ;
    btDefaultMotionState* myMotionState;
    btRigidBody* body;
    int sectionMap[OBS_GRID_SIZE][OBS_GRID_SIZE][RENDER_TUNNEL_SECTION_COUNT * 2];
    
    float x;
    float y;
    float z;
};
#endif