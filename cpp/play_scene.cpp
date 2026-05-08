/*
 * Copyright (C) Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdio>
#include "sfxman.hpp"
#include "anim.hpp"
#include "ascii_to_geom.hpp"
#include "game_consts.hpp"
#include "our_shader.hpp"
#include "play_scene.hpp"
#include "swift_call.h"
#include "util.hpp"
#include "common.hpp"
#include "welcome_scene.hpp"
#include "welcome_scene.hpp"
//#include "Tunnel"
#include "data/ascii_art.inl"
#include "data/cube_geom.inl"
#include "data/strings.inl"
#include "data/tunnel_geom.inl"
#include "data/teapot.inl"
#include <fstream>
#include <sstream>
#include <string>
#include "firebase/gma/ad_view.h"
#include "firebase/gma.h"
#include <unistd.h>

#define WALL_TEXTURE_SIZE 64


bool firsttouch=true;
static GLfloat TEAPOT_GEOM[240];
static GLushort TEAPOT_GEOM_INDICES[24];
static const int TEAPOT_GEOM_COLOR_OFFSET = 6 * sizeof(GLfloat);
static const int TEAPOT_GEOM_TEXCOORD_OFFSET = 3 * sizeof(GLfloat);
static const int TEAPOT_GEOM_STRIDE = 10 * sizeof(GLfloat);

// colors for menus
static const float MENUITEM_SEL_COLOR[] = { 1.0f, 1.0f, 0.0f };
static const float MENUITEM_COLOR[] = { 1.0f, 1.0f, 1.0f };
static glm::mat4 teapotModelMat(1.0f);

// obstacle colors
static const float OBS_COLORS[] = {
        0.0f, 0.0f, 0.0f, // style 0 (not used)
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f
};

// Snappy 3-note arpeggio; pitch and count rise with combo to reward chain collecting
static const char* TONE_BONUS[] = {
    "d25 f400. d30 f600. d35 f860.",
    "d25 f430. d30 f650. d35 f940.",
    "d25 f460. d30 f700. d35 f1020.",
    "d25 f500. d30 f760. d35 f1100.",
    "d20 f530. d25 f720. d25 f980. d30 f1360.",
    "d20 f570. d25 f780. d25 f1080. d30 f1500.",
    "d20 f610. d25 f840. d25 f1180. d30 f1640.",
    "d20 f650. d25 f900. d25 f1280. d30 f1800.",
    "d20 f700. d25 f980. d25 f1400. d30 f1960.",
};

PlayScene::PlayScene() : Scene() {
    // AAssetManager* assMgr;  // Android only
    hello= new Hello();
    mOurShader = NULL;
    mUseMove=false;
    mIsmenu=true;
    mTrivialShader = NULL;
    mTextRenderer = NULL;
    mShapeRenderer = NULL;
    mTeapotRenderer = new TexturedTeapotRender();
    mTeapotRenderer->Init(nullptr); // Android: Init(assMgr) — iOS uses bundle path
    mShipSteerX = mShipSteerZ = 0.0f;
    mFilteredSteerX = mFilteredSteerZ = 0.0f;
    mPlayerDir = glm::vec3(0.0f, 1.0f, 0.0f); // forward
    mDifficulty = 0;
    mUseCloudSave = false;
    mCubeGeom = NULL;
    mTunnelGeom = NULL;
    mObstacleCount = 0;
    mFirstObstacle = 0;
    mFirstSection = 0;
    mSteering = STEERING_NONE;
    mPointerId = -1;
    mPointerAnchorX = mPointerAnchorY = 0.0f;

    mWallTexture = NULL;

    memset(mMenuItemText, 0, sizeof(mMenuItemText));
    mMenuItemText[MENUITEM_UNPAUSE] = S_UNPAUSE;
    mMenuItemText[MENUITEM_QUIT] = S_QUIT;
    mMenuItemText[MENUITEM_START_OVER] = S_START_OVER;
    mMenuItemText[MENUITEM_RESUME] = S_RESUME;
    mMenuItemText[MENUITEM_BUYCOIN] = S_BUYCOIN;

    memset(mMenuItems, 0, sizeof(mMenuItems));
    mMenuItemCount = 0;

    mMenu = MENU_NONE;
    mMenuSel = 0;

    mSignText = NULL;
    mSignTimeLeft = 0.0f;

    mShowedHowto = false;
    mLifeGeom = NULL;

    mLives = PLAYER_LIVES;
    mGameOverExpire=1000000;
    mRollAngle = 0.0f;

    mPlayerSpeed = 0.0f;
    mBlinkingHeart = false;
    mGameStartTime = Clock();

    mBonusInARow = 0;
    mLastCrashSection = -1;

    mFrameClock.SetMaxDelta(MAX_DELTA_T);
    mLastAmbientBeepEmitted = 0;
    mMenuTouchActive = false;

    mCheckpointSignPending = false;
    SetScore(0);
    isLifeUpdated=false;

    mBonusFlashTime = 0.0f;
    mCrashShakeTime = 0.0f;
    mCrashFlashTime = 0.0f;
    mCamPos = glm::vec3(0.0f, 0.0f, 0.0f);
    mCamBankAngle = 0.0f;
    mClothVbo = 0; mClothIbo = 0; mClothProg = 0; mClothIndexCount = 0;
    mGlowProg = 0; mGlowVbo = 0;
    /*
     * where do I put the program???
     */
    SceneManager *mgr = SceneManager::GetInstance();
    const char *savePath = (mgr->mDocumentDataPath).c_str();//"/mnt/sdcard/com.google.example.games.tunnel.fix";
 
    int len = strlen(savePath) + strlen(SAVE_FILE_NAME) + 3;
    mSaveFileName = new char[len];
    strcpy(mSaveFileName, savePath);
 //   strcat(mSaveFileName, "/");
    strcat(mSaveFileName, SAVE_FILE_NAME);
    LOGD("Save file name: %s", mSaveFileName);
    LoadProgress();
    
    
    if (mSavedCheckpoint) {
        mUseMove=false;
        mIsmenu=true;
        // start with the menu that asks whether or not to start from the saved level
        // or start over from scratch
        ShowMenu(MENU_LEVEL);
    }else{
        mUseMove=true;
        mIsmenu=false;}
    
    
    
    
    
    InitBulletWorld();
}





void PlayScene::LoadProgress() {
    // try to load save file
    mSavedCheckpoint = 0;

    LOGD("Attempting to load: %s", mSaveFileName);
    FILE *f = fopen(mSaveFileName, "r");
    bool hasLocalFile = false;
    if (f) {
        hasLocalFile = true;
        LOGD("File found. Loading data.");
        std::ifstream myfile (mSaveFileName);
      //  if (1 != fscanf(f, "v1 %d", &mSavedCheckpoint)) {
        if( !myfile.is_open()){
          
            LOGE("Error parsing save file.");
            mSavedCheckpoint = 0;
        } else {
            
            std::string mystring;
            myfile >> mystring;
           // int num;
            std::istringstream iss(mystring);
            char* endPtr;
                int num = std::strtol(mystring.c_str(), &endPtr, 10);
            mSavedCheckpoint=num;
            LOGD("Loaded. Level = %d", mSavedCheckpoint);
            mSavedCheckpoint = (mSavedCheckpoint / LEVELS_PER_CHECKPOINT) * LEVELS_PER_CHECKPOINT;
            LOGD("Normalized check-point: level %d", mSavedCheckpoint);
        }
        fclose(f);
    } else {
        LOGD("Save file not present.");
    }

    // check cloud save.
    LOGD("Checking cloud save data.");
    if (true) {
        LOGD("No cloud save available because we are not signed in.");
        mUseCloudSave = false;
    }

    if (mUseCloudSave && hasLocalFile) {
        // since we're using cloud save, we can delete the local progress file
        LOGD("Since we're using cloud save, deleting local progress file %s", mSaveFileName);
        if (0 != remove(mSaveFileName)) {
            LOGW("WARNING: failed to remove local progress file.");
        }
    }

    LOGD("Final decision on starting level: %d", mSavedCheckpoint);
    LOGD("Final decision on whether to use cloud: %s", mUseCloudSave ? "USE CLOUD" :
            "DO NOT USE CLOUD (failed)");
}

void PlayScene::WriteSaveFile(int level) {
    LOGD("Saving progress (level %d) to file: %s", level, mSaveFileName);
   // FILE *f = fopen(mSaveFileName, "w");
    std::ofstream myfile (mSaveFileName);
    std::stringstream ss;
    ss<<level;
    std::string mystring=ss.str();
    myfile << mystring;
    if (!myfile) {
        LOGE("Error writing to save game file.");
        return;
    }
  //  fprintf(f, "v1 %d", level);
    //fclose(myfile);
    LOGD("Save file written.");
}

void PlayScene::SaveProgress() {
    if (mDifficulty <= mSavedCheckpoint) {
        // nothing to dole
        LOGD("No need to save level, current = %d, saved = %d", mDifficulty, mSavedCheckpoint);
        return;
    } else if (!IsCheckpointLevel()) {
        LOGD("Current level %d is not a checkpoint level. Nothing to save.", mDifficulty);
        return;
    }

    mSavedCheckpoint = mDifficulty;

    // Save state locally or to the cloud, depending on configuration:
    if (mUseCloudSave) {
        LOGD("Saving progress to the cloud: level %d", mDifficulty);
        /*
         * No where to save
         */
    } else {
        LOGD("Saving progress to LOCAL FILE: level %d", mDifficulty);
        WriteSaveFile(mDifficulty);
    }

    // Show a "checkpoint saved" sign when possible. We don't show it right away
    // because will already be showing the "Level N" sign, so we just set this flag
    // to remind us to show it right after.
    mCheckpointSignPending = true;
}

static unsigned char* _gen_wall_texture() {
    static unsigned char pixel_data[WALL_TEXTURE_SIZE * WALL_TEXTURE_SIZE * 3];
    unsigned char *p;
    int x, y;
    for (y = 0, p = pixel_data; y < WALL_TEXTURE_SIZE; y++) {
        for (x = 0; x < WALL_TEXTURE_SIZE; x++, p += 3) {
            p[0] = p[1] = p[2] = 128 + ((x > 2 && y > 2) ? Random(128) : 0);
            
        }
    }
    return pixel_data;
}

void PlayScene::OnStartGraphics() {
    // build shaders
    mOurShader = new OurShader();
    mOurShader->Compile();
    mTrivialShader = new TrivialShader();
    mTrivialShader->Compile();

    // build projection matrix
    UpdateProjectionMatrix();

    // build tunnel geometry
    mTunnelGeom = new SimpleGeom(
            new VertexBuf(TUNNEL_GEOM, sizeof(TUNNEL_GEOM),TUNNEL_GEOM_STRIDE),
            new IndexBuf(TUNNEL_GEOM_INDICES, sizeof(TUNNEL_GEOM_INDICES)));
    mTunnelGeom->vbuf->SetColorsOffset(TUNNEL_GEOM_COLOR_OFFSET);
    mTunnelGeom->vbuf->SetTexCoordsOffset(TUNNEL_GEOM_TEXCOORD_OFFSET);




    // build cube geometry (to draw obstacles)
   // mCubeGeom = new SimpleGeom(new VertexBuf(teapotPositions, sizeof(teapotPositions),6));
    mCubeGeom = new SimpleGeom(
                     new VertexBuf(CUBE_GEOM, sizeof(CUBE_GEOM),CUBE_GEOM_STRIDE));//,
                     //new IndexBuf(CUBE_GEOM_INDICES, sizeof(CUBE_GEOM_INDICES)));
    mCubeGeom->vbuf->SetColorsOffset(CUBE_GEOM_COLOR_OFFSET);
    mCubeGeom->vbuf->SetTexCoordsOffset(CUBE_GEOM_TEXCOORD_OFFSET);

    // make the wall texture
    mWallTexture = new Texture();
    mWallTexture->InitFromRawRGB(WALL_TEXTURE_SIZE, WALL_TEXTURE_SIZE, false,
            _gen_wall_texture());

    // reset frame clock so the animation doesn't jump
    mFrameClock.Reset();

    // life icon geometry
    mLifeGeom = AsciiArtToGeom(ART_LIFE, LIFE_ICON_SCALE);

    // create text renderer and shape renderer
    mTextRenderer = new TextRenderer(mTrivialShader);
    mShapeRenderer = new ShapeRenderer(mTrivialShader);

    InitCloth();
    InitGlow();

}

void PlayScene::OnKillGraphics() {
    CleanUp(&mTextRenderer);
    CleanUp(&mShapeRenderer);
    CleanUp(&mOurShader);
    CleanUp(&mTrivialShader);
    CleanUp(&mTunnelGeom);
    CleanUp(&mCubeGeom);
    CleanUp(&mWallTexture);
    CleanUp(&mLifeGeom);
    hello->~Hello();
    CleanupBulletWorld();
    if (mClothVbo)  { glDeleteBuffers(1, &mClothVbo);  mClothVbo  = 0; }
    if (mClothIbo)  { glDeleteBuffers(1, &mClothIbo);  mClothIbo  = 0; }
    if (mClothProg) { glDeleteProgram(mClothProg);      mClothProg = 0; }
    if (mGlowProg)  { glDeleteProgram(mGlowProg);       mGlowProg  = 0; }
    if (mGlowVbo)   { glDeleteBuffers(1, &mGlowVbo);   mGlowVbo   = 0; }
}

void PlayScene::DoFrame() {
    float deltaT = mFrameClock.ReadDelta();
    float previousY = mPlayerPos.y;

    // Ship visual world position (matches teapot render position)
    glm::vec3 shipVis(mPlayerPos.x, previousY + animPos, mPlayerPos.z);

    // ---- Spring-damped chase camera ----
    glm::vec3 camTarget = shipVis + glm::vec3(0.0f, -4.0f, 0.0f);
    float kCam = 1.0f - expf(-12.0f * deltaT);
    mCamPos += (camTarget - mCamPos) * kCam;

    // Lateral banking
    float steerErr = (mSteering ? mShipSteerX : 0.0f) - mPlayerPos.x;
    float bankTarget = Clamp(-steerErr * 0.04f, -0.45f, 0.45f);
    float kBank = 1.0f - expf(-8.0f * deltaT);
    mCamBankAngle += (bankTarget - mCamBankAngle) * kBank;

    // Camera micro-shake proportional to banking intensity
    float shakeAmp = fabsf(mCamBankAngle) * 0.45f;
    float t = Clock();
    glm::vec3 shake(sinf(t * 23.7f) * shakeAmp * 0.30f,
                    0.0f,
                    cosf(t * 31.1f) * shakeAmp * 0.30f);

    // Crash camera shake: high-frequency trauma shake that decays over 0.6 s
    if (mCrashShakeTime > 0.0f) {
        float trauma = mCrashShakeTime / 0.6f;   // 1 → 0
        float amp    = trauma * trauma * 2.2f;    // quadratic decay, peak ±2.2 units
        shake.x += sinf(t * 51.3f) * amp + cosf(t * 37.9f) * amp * 0.6f;
        shake.y  = sinf(t * 43.7f) * amp * 0.5f;
        shake.z += cosf(t * 59.1f) * amp + sinf(t * 29.3f) * amp * 0.6f;
    }

    // Build view matrix
    float rollTotal = mRollAngle + mCamBankAngle;
    glm::vec3 upVec = glm::vec3(-sinf(rollTotal), 0.0f, cosf(rollTotal));
    glm::vec3 lookTarget = shipVis + glm::vec3(0.0f, 6.0f, 0.0f);
    mViewMat = glm::lookAt(mCamPos + shake, lookTarget, upVec);

    // clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // render tunnel walls
    RenderTunnel();
    glEnable(GL_DEPTH_TEST);
    RenderObstacles();
    float angleInRadians = glm::radians(-100.0f); // Angle in radians
        glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        float angleInRadiansz = glm::radians(40.0f); // Angle in radians
        glm::vec3 rotationAxisz = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleInRadians, rotationAxis);
        glm::mat4 rotationMatrixz = glm::rotate(glm::mat4(1.0f), angleInRadiansz, rotationAxisz);

        if(k*Pos_step < 20.5) {
            animPos=k*Pos_step;
        }
        if(k*Scale_step < 0.1) {
            animScale=k*Scale_step;
        }
            glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(mPlayerPos.x, previousY + animPos,
                                                          mPlayerPos.z));

        modelMat = glm::scale(modelMat, glm::vec3(animScale, animScale, animScale));

        // Feed cloth-wave uniforms: normalized speed [0,1]
        float maxSpeed = PLAYER_SPEED + PLAYER_SPEED_INC_PER_LEVEL * 10.0f;
        mTeapotRenderer->SetShaderDynamics(Clock(),
            Clamp(fabsf(mPlayerSpeed) / maxSpeed, 0.0f, 1.0f));
        mTeapotRenderer->Render(mViewMat*modelMat*rotationMatrix*rotationMatrixz, mProjMat);

        // Cloth cape and engine glow trail (rendered while depth-test is still on)
        UpdateCloth(shipVis, deltaT);
        RenderCloth();
        RenderGlow(shipVis);

        // FOV pulse: widens 8° at full speed, narrows at rest
        float normSpeed = Clamp(fabsf(mPlayerSpeed) / maxSpeed, 0.0f, 1.0f);
        {
            SceneManager* mgr = SceneManager::GetInstance();
            float fov = glm::radians(RENDER_FOV + normSpeed * 8.0f);
            mProjMat = glm::perspective(fov, mgr->GetScreenAspect(),
                                        RENDER_NEAR_CLIP, RENDER_FAR_CLIP);
        }

        if(k<2000) {
            k++;
        }
    
  
    
    
    glEnable(GL_DEPTH_TEST);
    // render obstacles
    
    glEnable(GL_DEPTH_TEST);

    if (mMenu) {
        RenderMenu();
        // nothing more to do
       
       // mIsmenu=true;
        return;
    }
    mUseMove=true;

    // render HUD (lives, score, etc)
    RenderHUD();

    // deduct from the time remaining to remove a sign from the screen
    if (mSignText && mSignExpires) {
        mSignTimeLeft -= deltaT;
        if (mSignTimeLeft < 0.0f) {
            mSignText = NULL;
        }
    }
    if (mBonusFlashTime  > 0.0f) { mBonusFlashTime  -= deltaT; if (mBonusFlashTime  < 0) mBonusFlashTime  = 0; }
    if (mCrashShakeTime  > 0.0f) { mCrashShakeTime  -= deltaT; if (mCrashShakeTime  < 0) mCrashShakeTime  = 0; }
    if (mCrashFlashTime  > 0.0f) { mCrashFlashTime  -= deltaT; if (mCrashFlashTime  < 0) mCrashFlashTime  = 0; }

    // if a "saved checkpoint" sign pending? Can we show it right now?
    if (!mSignText && mCheckpointSignPending) {
        mCheckpointSignPending = false;
        ShowSign(S_CHECKPOINT_SAVED, SIGN_DURATION);
    }

    // did we already show the howto?
    if (!mShowedHowto && mDifficulty == 0) {
        mShowedHowto = true;
        ShowSign(S_HOWTO_WITHOUT_JOY, SIGN_DURATION);
    }

    // deduct from the time remaining on the blinking heart animation
    if (mBlinkingHeart && Clock() > mBlinkingHeartExpire) {
        mBlinkingHeart = false;
    }

    // update speed
    float targetSpeed = PLAYER_SPEED + PLAYER_SPEED_INC_PER_LEVEL * ((mDifficulty%6)+1);
    float accel = mPlayerSpeed >= 0.0f ? PLAYER_ACCELERATION_POSITIVE_SPEED :
            PLAYER_ACCELERATION_NEGATIVE_SPEED;
   if (mLives <= 0) {
       targetSpeed = 0.0f;
       purchasedelay=10;
    }
    mPlayerSpeed = Approach(mPlayerSpeed, targetSpeed, deltaT * accel);

    // Exponential moving average on raw steer input — smooths micro-jitter
    // while staying snappy enough to feel responsive (~20 Hz bandwidth).
    float ks = 1.0f - expf(-20.0f * deltaT);
    mFilteredSteerX += (mShipSteerX - mFilteredSteerX) * ks;
    mFilteredSteerZ += (mShipSteerZ - mFilteredSteerZ) * ks;

    // Move player: exponential approach to filtered steer target.
    // ~8 Hz gives a smooth, natural glide without feeling sluggish.
    if (mLives > 0) {
        float kl = 1.0f - expf(-8.0f * deltaT);
        if (mSteering == STEERING_JOY) {
            mPlayerPos.x += deltaT * mFilteredSteerX;
            mPlayerPos.z += deltaT * mFilteredSteerZ;
        } else {
            mPlayerPos.x += (mFilteredSteerX - mPlayerPos.x) * kl;
            mPlayerPos.z += (mFilteredSteerZ - mPlayerPos.z) * kl;
        }
    }
    mPlayerPos.y += deltaT * mPlayerSpeed;

    // make sure player didn't leave tunnel
    mPlayerPos.x = Clamp(mPlayerPos.x, PLAYER_MIN_X, PLAYER_MAX_X);
    mPlayerPos.z = Clamp(mPlayerPos.z, PLAYER_MIN_Z, PLAYER_MAX_Z);

   // float steerX = 0.0f, steerZ = 0.0f;
    // shift sections if needed
    ShiftIfNeeded();

    // generate more obstacles!
    GenObstacles();

    // Drift obstacles, then sync player ghost, then step broadphase once
    UpdateObstacleDrift(deltaT);
    {
        btTransform playerT;
        playerT.setIdentity();
        playerT.setOrigin(btVector3(mPlayerPos.x, mPlayerPos.y + 20.5f, mPlayerPos.z));
        mPlayerGhost->setWorldTransform(playerT);
        mBtWorld->stepSimulation(deltaT, 1);
    }

    // detect collisions
    DetectCollisions(previousY);

    // update ship's roll speed according to level
    static float roll_speeds[] = ROLL_SPEEDS;
    int count = sizeof(roll_speeds) / sizeof(float);
    float speed = roll_speeds[mDifficulty % count];
    mRollAngle += deltaT * speed;
    while (mRollAngle < 0) {
        mRollAngle += 2 * M_PI;
    }
    while (mRollAngle > 2 * M_PI) {
        mRollAngle -= 2 * M_PI;
    }
    int clock=Clock();
    // did the game expire?
    if (mLives <= 0 && clock > mGameOverExpire) {
        SceneManager::GetInstance()->RequestNewScene(new WelcomeScene(mApp));

    }

    // produce the ambient sound
    int soundPoint = (int)floor(mPlayerPos.y / (TUNNEL_SECTION_LENGTH/3));
    if (soundPoint % 3 != 0 && soundPoint > mLastAmbientBeepEmitted) {
        mLastAmbientBeepEmitted = soundPoint;
        SfxMan::GetInstance()->PlayTone(soundPoint % 2 ? TONE_AMBIENT_0 : TONE_AMBIENT_1);
    }
}

static float GetSectionCenterY(int i) {
    return (float)i * TUNNEL_SECTION_LENGTH;
}

static float GetSectionEndY(int i) {
    return GetSectionCenterY(i) + 0.5f * TUNNEL_SECTION_LENGTH;
}

static void _get_obs_color(int style, float *r, float *g, float *b) {
    style = Clamp(style, 1, 6);
    *r = OBS_COLORS[style * 3];
    *g = OBS_COLORS[style * 3 + 1];
    *b = OBS_COLORS[style * 3 + 2];
}

void PlayScene::RenderTunnel() {
    glm::mat4 modelMat;
    glm::mat4 mvpMat;
    int i, oi;

    mOurShader->BeginRender(mTunnelGeom->vbuf);
    mOurShader->SetTexture(mWallTexture);
    for (i = mFirstSection, oi = 0; i <= mFirstSection + RENDER_TUNNEL_SECTION_COUNT; ++i, ++oi) {
        float segCenterY = GetSectionCenterY(i);
        modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0.0, segCenterY, 0.0));
        mvpMat = mProjMat * mViewMat * modelMat;

        Obstacle *o = oi >= mObstacleCount ? NULL : GetObstacleAt(oi);

        // the point light is given in model coordinates, which is 0,0,0 is ok (center of
        // tunnel section)
        if (o) {
            float red, green, blue;
            _get_obs_color(o->style, &red, &green, &blue);
            mOurShader->EnablePointLight(glm::vec3(0.0, 0.0f, 0.0f), red, green, blue);
        } else {
            mOurShader->DisablePointLight();
        }

        // render tunnel section
        mOurShader->Render(mTunnelGeom->ibuf, &mvpMat);
    }
    mOurShader->EndRender();
}

void PlayScene::RenderObstacles() {
    int i;
    int r, c;
    float red, green, blue;
    glm::mat4 modelMat;
    glm::mat4 mvpMat;

    mOurShader->BeginRender(mCubeGeom->vbuf);
    mOurShader->SetTexture(mWallTexture);

    for (i = 0; i < mObstacleCount; i++) {
        Obstacle *o = GetObstacleAt(i);
        float posY = GetSectionCenterY(mFirstSection + i);
        if (o->style == Obstacle::STYLE_NULL) continue;

        // Look up live physics data for this section so rendering matches Bullet positions
        int section = mFirstSection + i;
        auto physIt = mSectionPhysics.find(section);

        for (r = 0; r < OBS_GRID_SIZE; r++) {
            for (c = 0; c < OBS_GRID_SIZE; c++) {
                bool isBonus = r == o->bonusRow && c == o->bonusCol;

                // Helper: get current drifted center from Bullet body, fall back to grid math
                auto bodyCenter = [&](int bIdx) -> glm::vec3 {
                    if (physIt != mSectionPhysics.end() &&
                        bIdx >= 0 && bIdx < (int)physIt->second.bodies.size() &&
                        physIt->second.bodies[bIdx]) {
                        btVector3 p = physIt->second.bodies[bIdx]->getWorldTransform().getOrigin();
                        return glm::vec3(p.getX(), p.getY(), p.getZ());
                    }
                    return o->GetBoxCenter(c, r, posY);
                };

                if (o->grid[c][r]) {
                    int bIdx = (physIt != mSectionPhysics.end())
                               ? physIt->second.bodyIdx[c][r] : -1;

                    // Build model matrix from live Bullet transform (pos + rotation)
                    // so blocks visually tumble as they fall and bounce.
                    if (bIdx >= 0 && physIt != mSectionPhysics.end() &&
                        bIdx < (int)physIt->second.bodies.size() &&
                        physIt->second.bodies[bIdx]) {
                        float m16[16];
                        physIt->second.bodies[bIdx]->getWorldTransform().getOpenGLMatrix(m16);
                        modelMat = glm::make_mat4(m16) *
                                   glm::scale(glm::mat4(1.0f), o->GetBoxSize(c, r));
                    } else {
                        glm::vec3 center = bodyCenter(bIdx);
                        modelMat = glm::translate(glm::mat4(1.0f), center);
                        modelMat = glm::scale(modelMat, o->GetBoxSize(c, r));
                    }
                    mvpMat = mProjMat * mViewMat * modelMat;

                    _get_obs_color(o->style, &red, &green, &blue);
                    mOurShader->SetTintColor(red, green, blue);
                    mOurShader->Render(&mvpMat);

                } else if (isBonus) {
                    int bIdx = (physIt != mSectionPhysics.end())
                               ? physIt->second.bonusBodyIdx : -1;
                    glm::vec3 center = bodyCenter(bIdx);

                    modelMat = glm::translate(glm::mat4(1.0f), center);
                    modelMat = glm::scale(modelMat, glm::vec3(OBS_BONUS_SIZE, OBS_BONUS_SIZE,
                                                               OBS_BONUS_SIZE));
                    modelMat = glm::rotate(modelMat, Clock() * 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                    teapotModelMat = modelMat;
                    mvpMat = mProjMat * mViewMat * modelMat;
                    mOurShader->SetTintColor(SineWave(0.8f, 1.0f, 0.5f, 0.0f),
                                             SineWave(0.8f, 1.0f, 0.5f, 0.0f),
                                             SineWave(0.8f, 1.0f, 0.5f, 0.0f));
                    mOurShader->Render(&mvpMat);
                }
            }
        }
    }
    mOurShader->EndRender();
}

void PlayScene::GenObstacles() {
    while (mObstacleCount < MAX_OBS) {
        // generate a new obstacle
        int index = (mFirstObstacle + mObstacleCount) % MAX_OBS;

        int section = mFirstSection + mObstacleCount;
        if (section < OBS_START_SECTION) {
            // generate an empty obstacle
            mObstacleCircBuf[index].Reset();
            mObstacleCircBuf[index].style = Obstacle::STYLE_NULL;
        } else {
            // At low difficulty, skip some sections to give the player breathing room:
            //   difficulty 0 → ~50% gaps, difficulty 1 → ~25% gaps, difficulty 2+ → 0%
            int skipDenom = (mDifficulty == 0) ? 2 : (mDifficulty == 1) ? 4 : 0;
            if (skipDenom > 0 && (rand() % skipDenom) == 0) {
                mObstacleCircBuf[index].Reset();
                mObstacleCircBuf[index].style = Obstacle::STYLE_NULL;
            } else {
                mObstacleGen.Generate(&mObstacleCircBuf[index]);
                AddBulletBodiesForSection(section, &mObstacleCircBuf[index]);
            }
        }
        mObstacleCount++;
    }
    
}

void PlayScene::ShiftIfNeeded() {
    // is it time to discard a section and shift forward?
    while (mPlayerPos.y > GetSectionEndY(mFirstSection) + SHIFT_THRESH) {
        RemoveBulletBodiesForSection(mFirstSection);

        // shift to the next tunnel section
        mFirstSection++;

        // discard obstacle corresponding to the deleted section
        if (mObstacleCount > 0) {
            // discarding first object (shifting) is easy because it's a circular buffer!
            mFirstObstacle = (mFirstObstacle + 1) % MAX_OBS;
            --mObstacleCount;
        }
    }
}

void PlayScene::UpdateMenuSelFromTouch(float x, float y) {
    float sh = SceneManager::GetInstance()->GetScreenHeight();
    int item = (int)floor(((sh-y) / sh) * (mMenuItemCount));
    mMenuSel = Clamp(item, 0, mMenuItemCount - 1);
}

void PlayScene::OnPointerDown(int pointerId, const struct PointerCoords *coords) {
    float x = coords->x, y = coords->y;
    if (mMenu) {
        if (coords->isScreen) {
            UpdateMenuSelFromTouch(x, y);
            
            mMenuTouchActive = true;
            
            OnPointerUp(pointerId, coords);
        }
    } else if (mSteering != STEERING_TOUCH) {
        mPointerId = pointerId;
        mPointerAnchorX = x;
        mPointerAnchorY = y;
        mShipAnchorX = mPlayerPos.x;
        mShipAnchorZ = mPlayerPos.z;
        mSteering = STEERING_TOUCH;
    }
}

void PlayScene::OnPointerUp(int pointerId, const struct PointerCoords *coords) {
    
    
    if (mMenu && mMenuTouchActive) {
        if (coords->isScreen) {
            mMenuTouchActive = false;
            HandleMenu(mMenuItems[mMenuSel]);
        }
    } else if (mSteering == STEERING_TOUCH && pointerId == mPointerId) {
        mSteering = STEERING_NONE;
        // Gently return steer target to current ship position so the
        // exponential smoother glides to a stop rather than snapping.
        mShipSteerX = mPlayerPos.x;
        mShipSteerZ = mPlayerPos.z;
    }
}

void PlayScene::OnPointerMove(int pointerId, const struct PointerCoords *coords) {
    float x = coords->x, y = coords->y;

    if (mMenu && mMenuTouchActive) {
        UpdateMenuSelFromTouch(x, y);
        return;
    }

    // On the very first move event after a finger-down, initialise the
    // rolling anchor without producing a steer delta.
    if (mSteering != STEERING_TOUCH) {
        mPointerId      = pointerId;
        mPointerAnchorX = x;
        mPointerAnchorY = y;
        mShipAnchorX    = mPlayerPos.x;
        mShipAnchorZ    = mPlayerPos.z;
        mSteering       = STEERING_TOUCH;
        return;
    }

    // Screen size used to normalise drag distance → tunnel units.
    float range = coords->isScreen
                  ? SceneManager::GetInstance()->GetScreenHeight()
                  : (coords->maxY - coords->minY);

    float dx = x - mPointerAnchorX;
    float dy = y - mPointerAnchorY;

    // Dead-zone: ignore sub-pixel jitter.
    const float DEAD = 1.5f;
    if (fabsf(dx) < DEAD && fabsf(dy) < DEAD) return;

    // Scale drag pixels → tunnel units and rotate by roll angle so the
    // ship always moves in the direction the player perceives as "sideways".
    float scale = TOUCH_CONTROL_SENSIVITY / range;
    float rotatedDx =  cosf(mRollAngle) * dx - sinf(mRollAngle) * dy;
    float rotatedDy =  sinf(mRollAngle) * dx + cosf(mRollAngle) * dy;

    // Accumulate delta onto the steer target and clamp to tunnel bounds.
    mShipSteerX = Clamp(mShipSteerX + rotatedDx * scale,
                        PLAYER_MIN_X, PLAYER_MAX_X);
    mShipSteerZ = Clamp(mShipSteerZ + rotatedDy * scale,
                        PLAYER_MIN_Z, PLAYER_MAX_Z);

    // Roll anchor forward so next event sees only the incremental delta.
    mPointerAnchorX = x;
    mPointerAnchorY = y;
}

void PlayScene::RenderHUD() {
    float aspect = SceneManager::GetInstance()->GetScreenAspect();
    glm::mat4 orthoMat = glm::ortho(0.0f, aspect, 0.0f, 1.0f);
    glm::mat4 modelMat;
    glm::mat4 mat;

    glDisable(GL_DEPTH_TEST);

    // render score digits
    int i, unit;
    static char score_str[6];
    int score = GetScore();
    for (i = 0, unit = 10000; i < 5; i++, unit /= 10) {
        score_str[i] = '0' + (score / unit) % 10;
    }
    score_str[i] = '\0';

    mTextRenderer->SetFontScale(SCORE_FONT_SCALE);
    mTextRenderer->RenderText(score_str, SCORE_POS_X, SCORE_POS_Y);

    // render current sign
    if (mSignText) {
        modelMat = glm::mat4(1.0f);
        float t = Clock() - mSignStartTime;
        if (t < SIGN_ANIM_DUR) {
            float scale = t / SIGN_ANIM_DUR;
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, scale, 1.0f));
        } else if (mSignTimeLeft < SIGN_ANIM_DUR) {
            float scale = mSignTimeLeft / SIGN_ANIM_DUR;
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, scale, 1.0f));
        }

        mTextRenderer->SetMatrix(modelMat);
        mTextRenderer->SetFontScale(SIGN_FONT_SCALE);
        if(mSignText==S_GOT_BONUS){
            mTextRenderer->RenderText(mSignText, aspect * 0.5f, 0.1f);
        }else{
            mTextRenderer->RenderText(mSignText, aspect * 0.5f, 0.5f);
        }
        
        mTextRenderer->ResetMatrix();
    }

    // render life icons
    glLineWidth(LIFE_LINE_WIDTH);
    float lifeX = LIFE_POS_X < 0.0f ? aspect + LIFE_POS_X : LIFE_POS_X;
    modelMat = glm::translate(glm::mat4(1.0), glm::vec3(lifeX, LIFE_POS_Y, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(1.0f, LIFE_SCALE_Y, 1.0f));
    int ubound = (mBlinkingHeart && BlinkFunc(0.2f)) ? mLives + 1 : mLives;
    for (int i = 0; i < ubound; i++) {
        mat = orthoMat * modelMat;
        mTrivialShader->RenderSimpleGeom(&mat, mLifeGeom);
        modelMat = glm::translate(modelMat, glm::vec3(LIFE_SPACING_X, 0.0f, 0.0f));
    }

    // Red screen vignette on crash
    if (mCrashFlashTime > 0.0f) {
        float tc = mCrashFlashTime / 0.45f;
        float ic = tc * tc * 0.6f;
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        mShapeRenderer->SetColor(ic, 0.0f, 0.0f);
        mShapeRenderer->RenderRect(aspect * 0.5f, 0.5f, aspect * 2.0f, 2.0f);
        glDisable(GL_BLEND);
    }

    // Golden screen flash on bonus pickup — additive blend so it brightens without overdraw clipping
    if (mBonusFlashTime > 0.0f) {
        float t = mBonusFlashTime / 0.35f;           // 1.0 → 0.0
        float intensity = t * t * 0.55f;              // ease-out: bright at peak, fades quickly
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);                  // additive: src + dst
        mShapeRenderer->SetColor(intensity,           // R
                                 intensity * 0.80f,   // G  → warm gold
                                 intensity * 0.12f);  // B
        mShapeRenderer->RenderRect(aspect * 0.5f, 0.5f, aspect * 2.0f, 2.0f);
        glDisable(GL_BLEND);
    }

    glEnable(GL_DEPTH_TEST);
}

void PlayScene::RenderMenu() {
    float aspect = SceneManager::GetInstance()->GetScreenAspect();
    glm::mat4 modelMat;
    glm::mat4 mat;
    mIsmenu=true;

    glDisable(GL_DEPTH_TEST);

    RenderBackgroundAnimation(mShapeRenderer);

    float scaleFactor = SineWave(1.0f, MENUITEM_PULSE_AMOUNT, MENUITEM_PULSE_PERIOD, 0.0f);

    int i;
    for (i = 0; i < mMenuItemCount; i++) {
        float thisFactor = (mMenuSel == i) ? scaleFactor : 1.0f;
        float y = 1.0f - (i + 1) / ((float)mMenuItemCount + 1);
        float x = aspect * 0.5f;
        mTextRenderer->SetFontScale(thisFactor * MENUITEM_FONT_SCALE);
        mTextRenderer->SetColor(mMenuSel == i ? MENUITEM_SEL_COLOR : MENUITEM_COLOR);
        mTextRenderer->RenderText(mMenuItemText[mMenuItems[i]], x, y);
    }
    mTextRenderer->ResetColor();

    glEnable(GL_DEPTH_TEST);
}

void PlayScene::DetectCollisions(float /*previousY*/) {
    // Handle life purchased via in-app purchase
    if (isLifeUpdated) {
        SceneManager *mgr = SceneManager::GetInstance();
        mLives = mgr->extraLife;
        ShowSign(S_LIFE_ADDED, SIGN_DURATION_GAME_OVER);
        mgr->extraLife = 0;
        isLifeUpdated = false;
    }

    // Game-over timeout path (unchanged)
    {
        int g = abs(mGameOverExpire - Clock());
        if (purchasedelay == 10 && g < 50000) {
            return;
        } else if (purchasedelay == 10) {
            purchasedelay = 0;
        }
    }

    // Query ghost overlapping objects populated by stepSimulation above
    bool hitObstacle = false;
    bool hitBonus    = false;
    int  bonusSection = -1;
    int  obsSection   = -1;

    int n = mPlayerGhost->getNumOverlappingObjects();
    for (int i = 0; i < n; i++) {
        btCollisionObject* obj = mPlayerGhost->getOverlappingObject(i);
        int section = obj->getUserIndex();   // which tunnel section
        int type    = obj->getUserIndex2();  // 0=obstacle, 1=bonus
        if (type == 1 && !hitBonus) {
            hitBonus    = true;
            bonusSection = section;
        } else if (type == 0 && !hitObstacle) {
            hitObstacle = true;
            obsSection  = section;
        }
    }

    // ---- Bonus collection ----
    if (hitBonus && bonusSection >= mFirstSection) {
        int circIdx = bonusSection - mFirstSection;
        if (circIdx < mObstacleCount) {
            Obstacle* o = GetObstacleAt(circIdx);
            if (o && o->HasBonus()) {
                // Remove the bonus body from the world immediately
                auto it = mSectionPhysics.find(bonusSection);
                if (it != mSectionPhysics.end()) {
                    int idx = it->second.bonusBodyIdx;
                    if (idx >= 0 && idx < (int)it->second.bodies.size() &&
                        it->second.bodies[idx]) {
                        mBtWorld->removeRigidBody(it->second.bodies[idx]);
                        delete it->second.bodies[idx];
                        delete it->second.shapes[idx];
                        it->second.bodies[idx] = nullptr;
                        it->second.shapes[idx] = nullptr;
                        it->second.bonusBodyIdx = -1;
                    }
                }
                o->DeleteBonus();
                ShowSign(S_GOT_BONUS, SIGN_DURATION_BONUS);
                AddScore(BONUS_POINTS);
                mBonusFlashTime = 0.35f;
                mBonusInARow++;
                if (mBonusInARow >= 10) mBonusInARow = 0;

                int score = GetScore();
                if (mDifficulty < score / SCORE_PER_LEVEL) {
                    mDifficulty = score / SCORE_PER_LEVEL;
                    ShowLevelSign();
                    mObstacleGen.SetDifficulty(mDifficulty);
                    SaveProgress();
                    SfxMan::GetInstance()->PlayTone(TONE_LEVEL_UP);
                } else {
                    int tone = (score % SCORE_PER_LEVEL) / BONUS_POINTS - 1;
                    tone = tone < 0 ? 0 :
                           tone >= (int)(sizeof(TONE_BONUS)/sizeof(char*)) ?
                           (int)(sizeof(TONE_BONUS)/sizeof(char*) - 1) : tone;
                    SfxMan::GetInstance()->PlayTone(TONE_BONUS[tone]);
                }
            }
        }
    }

    // ---- Obstacle collision ----
    if (!hitObstacle) return;

    // Immunity window: the ghost bounces back through the obstacle after a hit.
    // Skip processing until the blink animation expires so the same obstacle
    // can't deduct multiple lives in rapid succession.
    if (mBlinkingHeart) return;

    int targetSection = (obsSection >= mFirstSection) ? obsSection : mFirstSection;
    float obsCenter   = GetSectionCenterY(targetSection);

    if (mLives - 1 == 0) {
        BuyLifeInit();
        isLifeUpdated = true;
        return;
    }

    mLives--;
    mCrashShakeTime = 0.6f;
    mCrashFlashTime = 0.45f;
    if (mLives > 0) {
        ShowSign(S_OUCH, SIGN_DURATION);
        SfxMan::GetInstance()->PlayTone(TONE_CRASHED);
    } else {
        mIsmenu  = true;
        mUseMove = false;
        ShowSign(S_GAME_OVER, SIGN_DURATION_GAME_OVER);
        mGameOverExpire = Clock() + GAME_OVER_EXPIRE;
        SfxMan::GetInstance()->PlayTone(TONE_GAME_OVER);
    }

    // Place the ghost clearly PAST the obstacle's forward face so the negative
    // bounce speed can't immediately re-enter the same box.
    // ghost Y = obsCenter + halfBox + sphereRadius + 2-unit margin
    float safeGhostY = obsCenter + OBS_BOX_SIZE * 0.5f + 1.0f + 2.0f;
    mPlayerPos.y        = safeGhostY - 20.5f;
    mPlayerSpeed        = PLAYER_SPEED_AFTER_COLLISION;
    mBlinkingHeart      = true;
    mBlinkingHeartExpire = Clock() + BLINKING_HEART_DURATION;
    mLastCrashSection   = mFirstSection;

    // Move the ghost immediately to the cleared position so the next
    // stepSimulation call starts with no overlap.
    btTransform cleared;
    cleared.setIdentity();
    cleared.setOrigin(btVector3(mPlayerPos.x, safeGhostY, mPlayerPos.z));
    mPlayerGhost->setWorldTransform(cleared);
}

// ---------------------------------------------------------------------------
// Bullet Physics helpers
// ---------------------------------------------------------------------------

void PlayScene::InitBulletWorld() {
    mBtConfig     = new btDefaultCollisionConfiguration();
    mBtDispatcher = new btCollisionDispatcher(mBtConfig);
    mBtBroadphase = new btDbvtBroadphase();
    mGhostCallback = new btGhostPairCallback();
    mBtBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(mGhostCallback);
    mBtSolver = new btSequentialImpulseConstraintSolver();
    mBtWorld  = new btDiscreteDynamicsWorld(
        mBtDispatcher, mBtBroadphase, mBtSolver, mBtConfig);
    // Gravity perpendicular to tunnel (XZ plane) — rotated each frame with mRollAngle
    // so blocks always fall toward the current tunnel "floor".
    mBtWorld->setGravity(btVector3(0.0f, 0.0f, -22.0f));

    // Four infinite-plane static bodies = tunnel cross-section walls.
    // Normals point inward so dynamic obstacle blocks bounce off them.
    struct { btVector3 n; btScalar d; } planes[4] = {
        { btVector3( 1, 0, 0), TUNNEL_HALF_W },
        { btVector3(-1, 0, 0), TUNNEL_HALF_W },
        { btVector3( 0, 0, 1), TUNNEL_HALF_H },
        { btVector3( 0, 0,-1), TUNNEL_HALF_H },
    };
    for (int i = 0; i < 4; i++) {
        btStaticPlaneShape* s = new btStaticPlaneShape(planes[i].n, planes[i].d);
        btTransform t; t.setIdentity();
        btRigidBody::btRigidBodyConstructionInfo ci(0, nullptr, s, btVector3(0,0,0));
        btRigidBody* b = new btRigidBody(ci);
        b->setRestitution(0.55f);
        b->setFriction(0.3f);
        mBtWorld->addRigidBody(b, btBroadphaseProxy::StaticFilter,
                                  btBroadphaseProxy::DefaultFilter);
        mWallBodies[i] = b;
        mWallShapes[i] = s;
    }

    // Player ghost: sphere slightly smaller than half an obstacle cell so it
    // can fit through the gap when the player steers correctly.
    mPlayerShape = new btSphereShape(1.0f);
    mPlayerGhost = new btPairCachingGhostObject();
    mPlayerGhost->setCollisionShape(mPlayerShape);
    mPlayerGhost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

    btTransform startT;
    startT.setIdentity();
    startT.setOrigin(btVector3(0, 0, 0));
    mPlayerGhost->setWorldTransform(startT);

    mBtWorld->addCollisionObject(mPlayerGhost,
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
}

void PlayScene::CleanupBulletWorld() {
    for (int i = 0; i < 4; i++) {
        mBtWorld->removeRigidBody(mWallBodies[i]);
        delete mWallBodies[i];
        delete mWallShapes[i];
    }
    // Remove and delete all section bodies
    for (auto& kv : mSectionPhysics) {
        for (int i = 0; i < (int)kv.second.bodies.size(); i++) {
            if (kv.second.bodies[i]) {
                mBtWorld->removeRigidBody(kv.second.bodies[i]);
                delete kv.second.bodies[i];
            }
            if (kv.second.shapes[i]) {
                delete kv.second.shapes[i];
            }
        }
    }
    mSectionPhysics.clear();

    if (mPlayerGhost) {
        mBtWorld->removeCollisionObject(mPlayerGhost);
        delete mPlayerGhost;
        mPlayerGhost = nullptr;
    }
    delete mPlayerShape;
    delete mBtWorld;
    delete mBtSolver;
    delete mBtBroadphase; // also owns mGhostCallback after setInternalGhostPairCallback
    delete mBtDispatcher;
    delete mBtConfig;
}

void PlayScene::AddBulletBodiesForSection(int section, Obstacle* o) {
    if (!o || o->style == Obstacle::STYLE_NULL) return;

    float posY    = GetSectionCenterY(section);
    float halfBox = OBS_BOX_SIZE * 0.5f;
    btVector3 halfExtents(halfBox, halfBox, halfBox);

    SectionPhysics& sp = mSectionPhysics[section];
    sp.bonusBodyIdx = -1;
    for (int c = 0; c < OBS_GRID_SIZE; c++)
        for (int r = 0; r < OBS_GRID_SIZE; r++)
            sp.bodyIdx[c][r] = -1;

    for (int col = 0; col < OBS_GRID_SIZE; col++) {
        for (int row = 0; row < OBS_GRID_SIZE; row++) {
            bool isObstacle = o->grid[col][row];
            bool isBonus    = (row == o->bonusRow && col == o->bonusCol);
            if (!isObstacle && !isBonus) continue;

            glm::vec3 center = o->GetBoxCenter(col, row, posY);

            btBoxShape* shape = new btBoxShape(halfExtents);
            btTransform t;
            t.setIdentity();
            t.setOrigin(btVector3(center.x, center.y, center.z));

            // Bonus box stays kinematic (we spin it manually via clock).
            // Obstacle blocks are fully dynamic: they fall, tumble and bounce under gravity.
            float mass = isBonus ? 0.0f : 0.9f;
            btVector3 localInertia(0, 0, 0);
            if (mass > 0.0f) shape->calculateLocalInertia(mass, localInertia);

            btRigidBody::btRigidBodyConstructionInfo info(mass, nullptr, shape, localInertia);
            btRigidBody* body = new btRigidBody(info);
            body->setRestitution(0.45f);
            body->setFriction(0.5f);
            body->setActivationState(DISABLE_DEACTIVATION);
            body->setWorldTransform(t);
            body->setUserIndex(section);
            body->setUserIndex2(isBonus ? 1 : 0);

            if (mass == 0.0f) {
                // Bonus: kinematic so the clock-driven spin in RenderObstacles still works
                body->setCollisionFlags(body->getCollisionFlags() |
                                        btCollisionObject::CF_KINEMATIC_OBJECT);
                mBtWorld->addRigidBody(body,
                    btBroadphaseProxy::StaticFilter,
                    btBroadphaseProxy::CharacterFilter);
            } else {
                // Obstacle: dynamic — falls under gravity, bounces off tunnel walls
                // Give it a small random initial angular velocity so blocks start tumbling
                float ax = ((rand() % 200) - 100) * 0.03f;
                float az = ((rand() % 200) - 100) * 0.03f;
                body->setAngularVelocity(btVector3(ax, 0.0f, az));
                mBtWorld->addRigidBody(body,
                    btBroadphaseProxy::DefaultFilter,
                    btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::StaticFilter);
            }

            int idx = (int)sp.bodies.size();
            sp.bodies.push_back(body);
            sp.shapes.push_back(shape);

            if (isObstacle) sp.bodyIdx[col][row] = idx;
            if (isBonus)    sp.bonusBodyIdx = idx;
        }
    }
}

void PlayScene::RemoveBulletBodiesForSection(int section) {
    auto it = mSectionPhysics.find(section);
    if (it == mSectionPhysics.end()) return;

    SectionPhysics& sp = it->second;
    for (int i = 0; i < (int)sp.bodies.size(); i++) {
        if (sp.bodies[i]) {
            mBtWorld->removeRigidBody(sp.bodies[i]);
            delete sp.bodies[i];
        }
        if (sp.shapes[i]) {
            delete sp.shapes[i];
        }
    }
    mSectionPhysics.erase(it);
}

void PlayScene::UpdateObstacleDrift(float /*deltaT*/) {
    // Rotate physics gravity with the tunnel roll so blocks always fall toward
    // the current "floor" wall — as the tunnel spins, blocks slide dramatically
    // from wall to wall under real physics forces.
    const float G = 22.0f;
    mBtWorld->setGravity(btVector3(
        -sinf(mRollAngle) * G,
         0.0f,
        -cosf(mRollAngle) * G
    ));
}

// ---------------------------------------------------------------------------
// GL shader compile helper (local to this translation unit)
// ---------------------------------------------------------------------------
static GLuint CompileGLProgram(const char* vsrc, const char* fsrc) {
    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { glDeleteShader(s); return 0; }
        return s;
    };
    GLuint vs = compile(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc);
    if (!vs || !fs) { if (vs) glDeleteShader(vs); if (fs) glDeleteShader(fs); return 0; }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { glDeleteProgram(prog); return 0; }
    return prog;
}

// ---------------------------------------------------------------------------
// Cloth cape — position-based Verlet soft body
// ---------------------------------------------------------------------------
void PlayScene::InitCloth() {
    const float capeWidth = 1.4f;
    const float capeLen   = 3.0f;

    // Build initial node positions (relative; anchor row will be moved to ship each frame)
    for (int r = 0; r < CAPE_ROWS; r++) {
        for (int c = 0; c < CAPE_COLS; c++) {
            int idx = r * CAPE_COLS + c;
            float xOff = ((float)c / (CAPE_COLS - 1) - 0.5f) * capeWidth;
            float yOff = -(float)r / (CAPE_ROWS - 1) * capeLen;
            mClothNodes[idx].pos     = glm::vec3(xOff, yOff, 0.0f);
            mClothNodes[idx].prevPos = mClothNodes[idx].pos;
            mClothNodes[idx].fixed   = (r == 0);
        }
    }

    // Build springs (structural + shear + bending)
    auto addSpr = [&](int a, int b) {
        ClothSpring s;
        s.a = a; s.b = b;
        s.restLen = glm::length(mClothNodes[a].pos - mClothNodes[b].pos);
        mClothSprings.push_back(s);
    };
    for (int r = 0; r < CAPE_ROWS - 1; r++)
        for (int c = 0; c < CAPE_COLS; c++)
            addSpr(r * CAPE_COLS + c, (r + 1) * CAPE_COLS + c);        // vertical structural
    for (int r = 0; r < CAPE_ROWS; r++)
        for (int c = 0; c < CAPE_COLS - 1; c++)
            addSpr(r * CAPE_COLS + c, r * CAPE_COLS + c + 1);           // horizontal structural
    for (int r = 0; r < CAPE_ROWS - 1; r++)
        for (int c = 0; c < CAPE_COLS - 1; c++) {
            addSpr(r * CAPE_COLS + c,     (r + 1) * CAPE_COLS + c + 1); // shear
            addSpr(r * CAPE_COLS + c + 1, (r + 1) * CAPE_COLS + c);     // shear
        }
    for (int r = 0; r < CAPE_ROWS - 2; r++)
        for (int c = 0; c < CAPE_COLS; c++)
            addSpr(r * CAPE_COLS + c, (r + 2) * CAPE_COLS + c);         // bending vertical
    for (int r = 0; r < CAPE_ROWS; r++)
        for (int c = 0; c < CAPE_COLS - 2; c++)
            addSpr(r * CAPE_COLS + c, r * CAPE_COLS + c + 2);           // bending horizontal

    // Dynamic VBO: vec4 (xyz=pos, w=rowNorm) per node
    glGenBuffers(1, &mClothVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mClothVbo);
    glBufferData(GL_ARRAY_BUFFER, CAPE_COLS * CAPE_ROWS * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Static IBO: two triangles per quad
    std::vector<GLushort> idx;
    for (int r = 0; r < CAPE_ROWS - 1; r++) {
        for (int c = 0; c < CAPE_COLS - 1; c++) {
            GLushort tl = (GLushort)(r * CAPE_COLS + c);
            GLushort tr = tl + 1;
            GLushort bl = (GLushort)((r + 1) * CAPE_COLS + c);
            GLushort br = bl + 1;
            idx.push_back(tl); idx.push_back(bl); idx.push_back(tr);
            idx.push_back(bl); idx.push_back(br); idx.push_back(tr);
        }
    }
    mClothIndexCount = (int)idx.size();
    glGenBuffers(1, &mClothIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mClothIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLushort), idx.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Cloth shader
    const char* vsh =
        "attribute vec4 a_PosRow;"
        "uniform mat4 u_MVP;"
        "varying float v_Row;"
        "void main() {"
        "    gl_Position = u_MVP * vec4(a_PosRow.xyz, 1.0);"
        "    v_Row = a_PosRow.w;"
        "}";
    const char* fsh =
        "precision mediump float;"
        "varying float v_Row;"
        "void main() {"
        "    float alpha = 0.75 - v_Row * 0.55;"
        "    vec3 silk = mix(vec3(0.75,0.95,1.0), vec3(0.2,0.4,0.85), v_Row);"
        "    gl_FragColor = vec4(silk, alpha);"
        "}";
    mClothProg   = CompileGLProgram(vsh, fsh);
    mClothMvpLoc = glGetUniformLocation(mClothProg, "u_MVP");
    mClothPosLoc = glGetAttribLocation(mClothProg, "a_PosRow");
}

void PlayScene::UpdateCloth(glm::vec3 shipPos, float deltaT) {
    // 1. Pin anchor row (row 0) to ship attachment points
    const float capeWidth = 1.4f;
    for (int c = 0; c < CAPE_COLS; c++) {
        int idx = c; // row 0
        float xOff = ((float)c / (CAPE_COLS - 1) - 0.5f) * capeWidth;
        glm::vec3 anchor = shipPos + glm::vec3(xOff, 0.0f, 0.0f);
        mClothNodes[idx].pos     = anchor;
        mClothNodes[idx].prevPos = anchor;
    }

    // 2. Verlet integrate free nodes — wind drag proportional to forward speed
    float drag  = 0.98f;
    float speed = fabsf(mPlayerSpeed);
    glm::vec3 wind(0.0f, -speed * 0.0006f, 0.0f); // pushes cloth backward

    for (int i = CAPE_COLS; i < CAPE_COLS * CAPE_ROWS; i++) {
        glm::vec3 vel    = (mClothNodes[i].pos - mClothNodes[i].prevPos) * drag;
        mClothNodes[i].prevPos = mClothNodes[i].pos;
        mClothNodes[i].pos    += vel + wind * (deltaT * deltaT);
    }

    // 3. Constraint solve — 4 iterations
    for (int iter = 0; iter < 4; iter++) {
        for (auto& s : mClothSprings) {
            ClothNode& a = mClothNodes[s.a];
            ClothNode& b = mClothNodes[s.b];
            glm::vec3 delta = b.pos - a.pos;
            float len = glm::length(delta);
            if (len < 1e-5f) continue;
            float correction = (len - s.restLen) / len * 0.5f;
            glm::vec3 impulse = delta * correction;
            if (!a.fixed) a.pos += impulse;
            if (!b.fixed) b.pos -= impulse;
        }
    }

    // 4. Upload to VBO
    float vboData[CAPE_COLS * CAPE_ROWS * 4];
    for (int r = 0; r < CAPE_ROWS; r++) {
        float rowNorm = (float)r / (CAPE_ROWS - 1);
        for (int c = 0; c < CAPE_COLS; c++) {
            int idx = r * CAPE_COLS + c;
            vboData[idx * 4 + 0] = mClothNodes[idx].pos.x;
            vboData[idx * 4 + 1] = mClothNodes[idx].pos.y;
            vboData[idx * 4 + 2] = mClothNodes[idx].pos.z;
            vboData[idx * 4 + 3] = rowNorm;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, mClothVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vboData), vboData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlayScene::RenderCloth() {
    if (!mClothProg) return;
    glm::mat4 vp = mProjMat * mViewMat;

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(mClothProg);
    glUniformMatrix4fv(mClothMvpLoc, 1, GL_FALSE, glm::value_ptr(vp));

    glBindBuffer(GL_ARRAY_BUFFER, mClothVbo);
    glEnableVertexAttribArray(mClothPosLoc);
    glVertexAttribPointer(mClothPosLoc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mClothIbo);
    glDrawElements(GL_TRIANGLES, mClothIndexCount, GL_UNSIGNED_SHORT, nullptr);

    glDisableVertexAttribArray(mClothPosLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

// ---------------------------------------------------------------------------
// Engine glow — additive-blended elongated quads behind the ship
// ---------------------------------------------------------------------------
void PlayScene::InitGlow() {
    const char* vsh =
        "attribute vec4 a_PosAlpha;"
        "uniform mat4 u_MVP;"
        "varying float v_Alpha;"
        "void main() {"
        "    gl_Position = u_MVP * vec4(a_PosAlpha.xyz, 1.0);"
        "    v_Alpha = a_PosAlpha.w;"
        "}";
    const char* fsh =
        "precision mediump float;"
        "varying float v_Alpha;"
        "void main() {"
        "    gl_FragColor = vec4(0.35, 0.75, 1.0, v_Alpha);"
        "}";
    mGlowProg   = CompileGLProgram(vsh, fsh);
    mGlowMvpLoc = glGetUniformLocation(mGlowProg, "u_MVP");
    mGlowPosLoc = glGetAttribLocation(mGlowProg, "a_PosAlpha");

    glGenBuffers(1, &mGlowVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mGlowVbo);
    // 3 quads × 4 verts × 4 floats — allocate max size, fill each frame
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlayScene::RenderGlow(glm::vec3 shipPos) {
    if (!mGlowProg) return;

    // Glow layers: (halfWidth, topAlpha) pairs — inner=brightest, outer=dim
    struct Layer { float hw, alpha; };
    static const Layer layers[3] = { {0.12f, 0.90f}, {0.28f, 0.45f}, {0.50f, 0.18f} };

    float x  = shipPos.x;
    float z  = shipPos.z;
    float y0 = shipPos.y - 0.3f;   // just behind ship engine nozzle
    float y1 = shipPos.y - 7.0f;   // trail end

    // Build all 3 quads into one VBO upload (interleaved)
    float vbo[3 * 4 * 4]; // [layer][vert][xyzalpha]
    for (int l = 0; l < 3; l++) {
        float hw = layers[l].hw;
        float a0 = layers[l].alpha;
        float* q = vbo + l * 16;
        // top-left, top-right, bottom-left, bottom-right
        q[ 0]=x-hw; q[ 1]=y0; q[ 2]=z; q[ 3]=a0;
        q[ 4]=x+hw; q[ 5]=y0; q[ 6]=z; q[ 7]=a0;
        q[ 8]=x-hw*1.8f; q[ 9]=y1; q[10]=z; q[11]=0.0f;
        q[12]=x+hw*1.8f; q[13]=y1; q[14]=z; q[15]=0.0f;
    }

    glm::mat4 vp = mProjMat * mViewMat;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive blend for glow bloom

    glUseProgram(mGlowProg);
    glUniformMatrix4fv(mGlowMvpLoc, 1, GL_FALSE, glm::value_ptr(vp));

    glBindBuffer(GL_ARRAY_BUFFER, mGlowVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vbo), vbo);
    glEnableVertexAttribArray(mGlowPosLoc);
    glVertexAttribPointer(mGlowPosLoc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

    // Draw each quad as two triangles (no IBO: use triangle_strip)
    for (int l = 0; l < 3; l++) {
        glDrawArrays(GL_TRIANGLE_STRIP, l * 4, 4);
    }

    glDisableVertexAttribArray(mGlowPosLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

bool PlayScene::OnBackKeyPressed() {
    if (mMenu) {
        // reset frame clock so that the animation doesn't jump:
        mFrameClock.Reset();

        // leave menu
        ShowMenu(MENU_NONE);
    } else {
        // enter pause menu
       //  BuyLife();
        ShowMenu(MENU_PAUSE);
    }
    return true;
}

bool PlayScene::IsMenu() {
    
    return mIsmenu;
}
bool PlayScene::UseMove() {
    
    return mUseMove;
}
bool PlayScene::BuyLifeInit() {
    if (mMenu) {
        // reset frame clock so that the animation doesn't jump:
        mFrameClock.Reset();

        // leave menu
        ShowMenu(MENU_NONE);
    } else {
        // enter pause menu
        ShowAd();
        ShowMenu(MENU_PAUSE);
    }
    return true;
}


void PlayScene::OnJoy(float joyX, float joyY) {
    if (!mSteering || mSteering == STEERING_JOY) {
        float deltaX = joyX * JOYSTICK_CONTROL_SENSIVITY;
        float deltaY = joyY * JOYSTICK_CONTROL_SENSIVITY;
        float rotatedDx = cos(-mRollAngle) * deltaX - sin(-mRollAngle) * deltaY;
        float rotatedDy = sin(-mRollAngle) * deltaX + cos(-mRollAngle) * deltaY;
        mShipSteerX = rotatedDx;
        mShipSteerZ = -rotatedDy;
        mSteering = STEERING_JOY;

        // If player is going faster than the reference speed, PLAYER_SPEED, adjust it.
        // This makes the steering react faster as the ship accelerates in more difficult
        // levels.
        if (mPlayerSpeed > PLAYER_SPEED) {
            mShipSteerX *= mPlayerSpeed / PLAYER_SPEED;
            mShipSteerZ *= mPlayerSpeed / PLAYER_SPEED;
        }
    }
}

void PlayScene::OnKeyDown(int keyCode) {
    if (mMenu) {
        if (keyCode == OURKEY_UP) {
            mMenuSel = mMenuSel > 0 ? mMenuSel - 1 : mMenuSel;
            HandleMenu(mMenuItems[mMenuSel]);
        }
        else if (keyCode == OURKEY_DOWN) {
            mMenuSel = mMenuSel + 1 < mMenuItemCount ? mMenuSel + 1 : mMenuSel;
            HandleMenu(mMenuItems[mMenuSel]);
        }
        else if (keyCode == OURKEY_ENTER) {
            HandleMenu(mMenuItems[mMenuSel]);
        }
    }
}

void PlayScene::ShowMenu(int menu) {
    mMenu = menu;
    mMenuSel = 0;
    switch (menu) {
        case MENU_PAUSE:
            mMenuItems[0] = MENUITEM_UNPAUSE;
            mMenuItems[1] = MENUITEM_QUIT;
            mMenuItems[2] = MENUITEM_BUYCOIN;
            mMenuItemCount = 3;
            break;
        case MENU_LEVEL:
            mMenuItems[0] = MENUITEM_RESUME;
            mMenuItems[1] = MENUITEM_START_OVER;
            mMenuItemCount = 2;
            break;
        default:
            // since we're leaving the menu, reset the frame clock to avoid a skip
            // in the animation
            mFrameClock.Reset();
    }
}

void PlayScene::HandleMenu(int menuItem) {
    switch (menuItem) {
        case MENUITEM_QUIT:
            SceneManager::GetInstance()->RequestNewScene(new WelcomeScene(mApp));
            break;
        case MENUITEM_UNPAUSE:
            ShowMenu(MENU_NONE);
            mUseMove=true;
            mIsmenu=false;
            break;
        case MENUITEM_RESUME:
            // resume from saved level
            mDifficulty = (mSavedCheckpoint / LEVELS_PER_CHECKPOINT) * LEVELS_PER_CHECKPOINT;
            SetScore(SCORE_PER_LEVEL * mDifficulty);
            mObstacleGen.SetDifficulty(mDifficulty);
            ShowLevelSign();
            ShowMenu(MENU_NONE);
            mUseMove=true;
            mIsmenu=false;
            break;
        case MENUITEM_START_OVER:
            // start over from scratch
            ShowMenu(MENU_NONE);
            mUseMove=true;
            mIsmenu=false;
            break;
        case MENUITEM_BUYCOIN:
            BuyConsumableC();
            ShowMenu(MENU_PAUSE);
            break;
    }
}

void PlayScene::ShowLevelSign() {
    static char level_str[] = "LEVEL XX";
    int level = mDifficulty + 1;
    level_str[6] = '0' + ((level > 9) ? (level / 10) % 10 : level % 10);
    level_str[7] = (level > 9) ? ('0' + level % 10) : '\0';
    level_str[8] = '\0';
    ShowSign(level_str, SIGN_DURATION);
}

void PlayScene::OnPause() {
    if (mMenu == MENU_NONE) {
        ShowMenu(MENU_PAUSE);
    }
}

void PlayScene::OnScreenResized(int width, int height) {
    UpdateProjectionMatrix();
}

void PlayScene::UpdateProjectionMatrix() {
    SceneManager *mgr = SceneManager::GetInstance();
    mProjMat = glm::perspective(RENDER_FOV, mgr->GetScreenAspect(), RENDER_NEAR_CLIP,
            RENDER_FAR_CLIP);
}

void PlayScene::BuyLife() 
{

    BuyConsumableC();
/*JNIEnv * env=mApp->activity->env;

mApp->activity->vm->AttachCurrentThread(&env, NULL);

jobject lNativeActivity = mApp->activity->clazz;
jclass intentClass = env->FindClass("android/content/Intent");
jstring actionString =env->NewStringUTF("com.joyholdings.tunnel.BillingActivity");
jmethodID newIntent = env->GetMethodID(intentClass, "<init>", "()V");
jobject intent = env->AllocObject(intentClass);
env->CallVoidMethod(intent, newIntent);
jmethodID setAction = env->GetMethodID(intentClass, "setAction","(Ljava/lang/String;)Landroid/content/Intent;");
env->CallObjectMethod(intent, setAction, actionString);
jclass activityClass = env->FindClass("android/app/Activity");
jmethodID startActivity = env->GetMethodID(activityClass,"startActivity", "(Landroid/content/Intent;)V");
jobject intentObject = env->NewObject(intentClass,newIntent);
env->CallObjectMethod(intentObject, setAction,actionString);
env->CallVoidMethod( lNativeActivity, startActivity, intentObject);
env->DeleteLocalRef( intent ); 
env->DeleteLocalRef( intentObject); 
env->DeleteLocalRef( activityClass );
env->DeleteLocalRef( intentClass );


mApp->activity->vm->DetachCurrentThread();*/
}



/*extern "C" JNIEXPORT jstring JNICALL
Java_com_joyholdings_tunnel_BillingActivity_UpdateLife( JNIEnv* env,
                                                  jobject thiz,jint life_nos )
{
    
life=life_nos;
isLifeUpdated=true;
return (*env).NewStringUTF("updated");
}*/
void PlayScene::InitTeapot(){
  int j=0;
   int k=0;
  for (int i=0;i<24;i++ )
  {
    
    TEAPOT_GEOM[j]=(GLfloat)teapotPositions[k]*0.5;
    TEAPOT_GEOM[j+1]=(GLfloat)teapotPositions[k+1]*0.5;
    TEAPOT_GEOM[j+2]=(GLfloat)teapotPositions[k+2]*0.5;
    TEAPOT_GEOM[j+3]=0.7f;//(GLfloat)teapotTexCoords[k];
    TEAPOT_GEOM[j+4]=0.7f;//(GLfloat)teapotTexCoords[k+1];
    TEAPOT_GEOM[j+5]=0.7f;//(GLfloat)teapotTexCoords[k+2];
    TEAPOT_GEOM[j+6]=0.7f;
    TEAPOT_GEOM[j+7]=0.7f;
    TEAPOT_GEOM[j+8]=0.7f;
    TEAPOT_GEOM[j+9]=1.0f;
    j+=10;
    k+=8;
    
  }

for (int i=0;i<24;i++ )
  { TEAPOT_GEOM_INDICES[i]=(GLushort)teapotIndices[i];
  }
}
