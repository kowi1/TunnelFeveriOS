#pragma once

#include <vector>
#include <OpenGLES/ES2/gl.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "android_assetmanager.hpp"

// Bone indices (must match shader if-else chain)
enum BoneId { BONE_ROOT=0, BONE_TORSO, BONE_HEAD, BONE_LARM, BONE_RARM, BONE_LLEG, BONE_RLEG, BONE_COUNT };

class CharacterRenderer {
public:
    CharacterRenderer();
    ~CharacterRenderer();

    void Init(AAssetManager* amgr);
    void Unload();

    // Call each frame before Render. bank = signed lean angle (radians),
    // speed = 0..1 normalised speed, stridePhase = running phase 0..2pi.
    void Update(glm::vec3 rootPos, float bankAngle, float speed, float stridePhase, float breathPhase);
    void Render(glm::mat4 viewProj);

private:
    // Vertex: 10 floats — position(3), normal(3), color(3), boneIdx(1)
    static const int FLOATS_PER_VERT = 10;

    GLuint mVbo;
    GLuint mIbo;
    GLuint mProg;
    int    mIndexCount;

    // Uniform locations
    GLint mLocVP;           // uVP  — view-projection matrix
    GLint mLocBones[BONE_COUNT]; // uBones[i] — 4x4 bone transform

    // Attribute locations
    GLint mAttrPos;
    GLint mAttrNorm;
    GLint mAttrColor;
    GLint mAttrBone;

    // Current bone world matrices (updated every frame)
    glm::mat4 mBoneMatrices[BONE_COUNT];

    // Build static mesh geometry
    void BuildMesh();

    // Append a transformed box for one body part
    void AddBox(std::vector<float>& verts, std::vector<uint16_t>& indices,
                glm::vec3 center, glm::vec3 halfExt, glm::vec3 color, int boneIdx);
};
