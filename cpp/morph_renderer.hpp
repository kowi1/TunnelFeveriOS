#pragma once
#include <OpenGLES/ES2/gl.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "android_assetmanager.hpp"

// Morph-target keyframe renderer for the animated humanoid character.
// Blends pairwise between 6 pre-baked skeletal poses via vertex shader mix().
// Frames: 0=NEUTRAL 1=RUN_A 2=RUN_B 3=BANK_L 4=BANK_R 5=CROUCH
class MorphRenderer {
public:
    static const int FRAME_COUNT = 6;

    MorphRenderer();
    ~MorphRenderer();

    void Init(AAssetManager* amgr);
    void Unload();

    // Update animation state. speed 0..1, bankAngle signed radians.
    void Update(float speed, float bankAngle, float deltaT);

    // Render. modelOnly = model transform without view/proj (for world-space lighting).
    void Render(glm::mat4 mvp, glm::mat4 modelOnly, glm::vec3 camPos);

private:
    GLuint mFrameVbo[FRAME_COUNT];
    GLuint mIbo;
    int    mIndexCount;
    GLuint mProg;

    GLint mLocMVP, mLocModel, mLocBlend, mLocLight, mLocCam;

    int   mSrcFrame, mDstFrame;
    float mBlend;
    float mPhase;
    float mBankBlend;
};
