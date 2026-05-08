#include "morph_renderer.hpp"
#include "data/character_anim.inl"   // gCharPos/Nor 0..5, gCharIdx, kChar*
#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
// Vertex shader: blend two morph frames, output world pos + normal for frag.
// ---------------------------------------------------------------------------
static const char* kVsh = R"GLSL(
attribute highp vec3  aPosA;
attribute highp vec3  aNorA;
attribute highp vec3  aPosB;
attribute highp vec3  aNorB;

uniform highp   mat4  uMVP;
uniform highp   mat4  uModel;     // model-only (no view/proj) for world pos
uniform mediump float uBlend;

varying highp   vec3  vWorldPos;
varying mediump vec3  vNorm;

void main() {
    highp vec3 pos  = mix(aPosA, aPosB, uBlend);
    highp vec3 norm = normalize(mix(aNorA, aNorB, uBlend));
    gl_Position = uMVP * vec4(pos, 1.0);
    vWorldPos   = (uModel * vec4(pos, 1.0)).xyz;
    vNorm       = normalize(mat3(uModel) * norm);
}
)GLSL";

// ---------------------------------------------------------------------------
// Fragment shader: diffuse + specular + rim lighting, R/G/Y palette.
// ---------------------------------------------------------------------------
static const char* kFsh = R"GLSL(
precision mediump float;
varying vec3 vWorldPos;
varying vec3 vNorm;

uniform vec3 uCamPos;
uniform vec3 uLight;

void main() {
    vec3 norm     = normalize(vNorm);
    vec3 lightDir = normalize(-uLight);
    vec3 viewDir  = normalize(uCamPos - vWorldPos);

    // R / G / Y normal-direction palette
    vec3 n = abs(norm);
    vec3 palette =
        n.x * vec3(1.00, 0.05, 0.00)   // X faces → vivid red
      + n.y * vec3(0.05, 1.00, 0.05)   // Y faces → vivid green
      + n.z * vec3(1.00, 0.88, 0.00);  // Z faces → vivid yellow
    palette = clamp(palette * 1.35, 0.0, 1.0);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3  lit  = palette * (0.28 + diff * 0.88);

    // Specular (Blinn-Phong)
    vec3  h    = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, h), 0.0), 32.0) * 0.55;

    // Rim / Fresnel — electric outline on silhouette edges
    float rim  = 1.0 - max(dot(norm, viewDir), 0.0);
    rim = pow(rim, 2.8) * 0.70;
    vec3 rimCol = vec3(0.75, 0.95, 1.00) * rim;  // cool blue-white rim

    gl_FragColor = vec4(clamp(lit + spec + rimCol, 0.0, 1.0), 1.0);
}
)GLSL";

// ---------------------------------------------------------------------------
static GLuint CompileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}
static GLuint LinkProgram(const char* vsh, const char* fsh) {
    GLuint v = CompileShader(GL_VERTEX_SHADER,   vsh);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fsh);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glBindAttribLocation(p, 0, "aPosA");
    glBindAttribLocation(p, 1, "aNorA");
    glBindAttribLocation(p, 2, "aPosB");
    glBindAttribLocation(p, 3, "aNorB");
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ---------------------------------------------------------------------------
MorphRenderer::MorphRenderer()
    : mIbo(0), mIndexCount(0), mProg(0),
      mLocMVP(-1), mLocBlend(-1), mLocLight(-1),
      mSrcFrame(0), mDstFrame(0), mBlend(0.0f),
      mPhase(0.0f), mBankBlend(0.0f)
{
    memset(mFrameVbo, 0, sizeof(mFrameVbo));
}
MorphRenderer::~MorphRenderer() { Unload(); }

void MorphRenderer::Init(AAssetManager* /*amgr*/) {
    mProg     = LinkProgram(kVsh, kFsh);
    mLocMVP   = glGetUniformLocation(mProg, "uMVP");
    mLocModel = glGetUniformLocation(mProg, "uModel");
    mLocBlend = glGetUniformLocation(mProg, "uBlend");
    mLocLight = glGetUniformLocation(mProg, "uLight");
    mLocCam   = glGetUniformLocation(mProg, "uCamPos");

    // Build interleaved VBO per frame: [px,py,pz, nx,ny,nz] * kCharVertCount
    float* buf = new float[kCharVertCount * 6];
    float* posTbl[FRAME_COUNT] = { gCharPos0,gCharPos1,gCharPos2,
                                   gCharPos3,gCharPos4,gCharPos5 };
    float* norTbl[FRAME_COUNT] = { gCharNor0,gCharNor1,gCharNor2,
                                   gCharNor3,gCharNor4,gCharNor5 };
    glGenBuffers(FRAME_COUNT, mFrameVbo);
    for (int f = 0; f < FRAME_COUNT; ++f) {
        for (int i = 0; i < kCharVertCount; ++i) {
            buf[i*6+0]=posTbl[f][i*3+0]; buf[i*6+1]=posTbl[f][i*3+1];
            buf[i*6+2]=posTbl[f][i*3+2]; buf[i*6+3]=norTbl[f][i*3+0];
            buf[i*6+4]=norTbl[f][i*3+1]; buf[i*6+5]=norTbl[f][i*3+2];
        }
        glBindBuffer(GL_ARRAY_BUFFER, mFrameVbo[f]);
        glBufferData(GL_ARRAY_BUFFER, kCharVertCount*6*sizeof(float), buf, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete[] buf;

    mIndexCount = kCharIdxCount;
    glGenBuffers(1, &mIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, kCharIdxCount*sizeof(unsigned short),
                 gCharIdx, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MorphRenderer::Unload() {
    if (mProg) { glDeleteProgram(mProg); mProg=0; }
    if (mIbo)  { glDeleteBuffers(1,&mIbo); mIbo=0; }
    if (mFrameVbo[0]) { glDeleteBuffers(FRAME_COUNT,mFrameVbo);
                        memset(mFrameVbo,0,sizeof(mFrameVbo)); }
}

// ---------------------------------------------------------------------------
void MorphRenderer::Update(float speed, float bankAngle, float deltaT) {
    float cycleRate = speed * 2.8f;
    mPhase += deltaT * cycleRate;
    if (mPhase > 1.0f) mPhase -= 1.0f;

    // Smooth bank value
    float bankTarget = bankAngle / 0.45f;
    float kb = 1.0f - expf(-6.0f * deltaT);
    mBankBlend += (bankTarget - mBankBlend) * kb;
    mBankBlend = mBankBlend < -1.f ? -1.f : (mBankBlend > 1.f ? 1.f : mBankBlend);
    float absBk = mBankBlend < 0 ? -mBankBlend : mBankBlend;

    if (absBk > 0.15f) {
        mSrcFrame = (speed > 0.05f) ? (mPhase < 0.5f ? 1 : 2) : 0;
        mDstFrame = (mBankBlend < 0) ? 3 : 4;
        mBlend    = absBk;
    } else if (speed > 0.05f) {
        // 3-frame run cycle: NEUTRAL→RUN_A→RUN_B→NEUTRAL
        if      (mPhase < 0.33f) { mSrcFrame=0; mDstFrame=1; mBlend=mPhase/0.33f; }
        else if (mPhase < 0.66f) { mSrcFrame=1; mDstFrame=2; mBlend=(mPhase-0.33f)/0.33f; }
        else                     { mSrcFrame=2; mDstFrame=0; mBlend=(mPhase-0.66f)/0.34f; }
    } else {
        // Idle hover: breathe between NEUTRAL and CROUCH
        mSrcFrame = 0; mDstFrame = 5;
        mBlend = 0.18f + sinf(mPhase * 6.283f) * 0.12f;
    }
}

// ---------------------------------------------------------------------------
void MorphRenderer::Render(glm::mat4 mvp, glm::mat4 modelOnly, glm::vec3 camPos) {
    if (!mProg || !mIbo || !mFrameVbo[0]) return;

    glUseProgram(mProg);
    glUniformMatrix4fv(mLocMVP,   1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(mLocModel, 1, GL_FALSE, glm::value_ptr(modelOnly));
    glUniform1f(mLocBlend, mBlend);
    glUniform3f(mLocLight, 100.f, -200.f, -600.f);
    glUniform3f(mLocCam, camPos.x, camPos.y, camPos.z);

    int stride = 6 * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER, mFrameVbo[mSrcFrame]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, mFrameVbo[mDstFrame]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2); glEnableVertexAttribArray(3);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, (void*)0);

    glDisableVertexAttribArray(0); glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2); glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
