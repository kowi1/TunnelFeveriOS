#include "character_renderer.hpp"
#include <vector>
#include <cmath>
#include <cstring>

// --------------------------------------------------------------------------
// GLSL source — ES 2.0, if-else bone dispatch (max 7 bones)
// --------------------------------------------------------------------------
static const char* kCharVsh = R"GLSL(
attribute highp   vec3  aPos;
attribute mediump vec3  aNorm;
attribute lowp    vec3  aColor;
attribute lowp    float aBone;

uniform highp mat4 uVP;
uniform highp mat4 uBone0;
uniform highp mat4 uBone1;
uniform highp mat4 uBone2;
uniform highp mat4 uBone3;
uniform highp mat4 uBone4;
uniform highp mat4 uBone5;
uniform highp mat4 uBone6;

varying lowp    vec3  vColor;
varying mediump vec3  vNorm;
varying mediump vec3  vPos;

void main() {
    int b = int(aBone + 0.5);
    highp mat4 bMat;
    if      (b == 0) bMat = uBone0;
    else if (b == 1) bMat = uBone1;
    else if (b == 2) bMat = uBone2;
    else if (b == 3) bMat = uBone3;
    else if (b == 4) bMat = uBone4;
    else if (b == 5) bMat = uBone5;
    else             bMat = uBone6;

    highp vec4 worldPos = bMat * vec4(aPos, 1.0);
    gl_Position = uVP * worldPos;
    vNorm  = mat3(bMat) * aNorm;
    vPos   = worldPos.xyz;
    vColor = aColor;
}
)GLSL";

static const char* kCharFsh = R"GLSL(
precision mediump float;
varying vec3 vColor;
varying vec3 vNorm;
varying vec3 vPos;

void main() {
    vec3 lightDir = normalize(vec3(0.4, 1.0, 0.6));
    float diff = max(dot(normalize(vNorm), lightDir), 0.0);
    vec3 lit = vColor * (0.35 + diff * 0.85);
    // Small specular pop
    vec3 h = normalize(lightDir - normalize(vPos));
    float spec = pow(max(dot(normalize(vNorm), h), 0.0), 24.0) * 0.4;
    gl_FragColor = vec4(clamp(lit + spec, 0.0, 1.0), 1.0);
}
)GLSL";

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------
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
    glAttachShader(p, v);
    glAttachShader(p, f);
    glBindAttribLocation(p, 0, "aPos");
    glBindAttribLocation(p, 1, "aNorm");
    glBindAttribLocation(p, 2, "aColor");
    glBindAttribLocation(p, 3, "aBone");
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// --------------------------------------------------------------------------
// CharacterRenderer
// --------------------------------------------------------------------------
CharacterRenderer::CharacterRenderer()
    : mVbo(0), mIbo(0), mProg(0), mIndexCount(0),
      mLocVP(-1), mAttrPos(0), mAttrNorm(1), mAttrColor(2), mAttrBone(3)
{
    memset(mLocBones, -1, sizeof(mLocBones));
    for (int i = 0; i < BONE_COUNT; ++i)
        mBoneMatrices[i] = glm::mat4(1.0f);
}

CharacterRenderer::~CharacterRenderer() { Unload(); }

void CharacterRenderer::Init(AAssetManager* /*amgr*/) {
    mProg = LinkProgram(kCharVsh, kCharFsh);

    mLocVP = glGetUniformLocation(mProg, "uVP");
    const char* boneNames[BONE_COUNT] = {
        "uBone0","uBone1","uBone2","uBone3","uBone4","uBone5","uBone6"
    };
    for (int i = 0; i < BONE_COUNT; ++i)
        mLocBones[i] = glGetUniformLocation(mProg, boneNames[i]);

    BuildMesh();
}

void CharacterRenderer::Unload() {
    if (mVbo)  { glDeleteBuffers(1, &mVbo);  mVbo  = 0; }
    if (mIbo)  { glDeleteBuffers(1, &mIbo);  mIbo  = 0; }
    if (mProg) { glDeleteProgram(mProg);      mProg = 0; }
}

// --------------------------------------------------------------------------
// AddBox — appends one CCW-wound box in bone-local space
// Each face: 4 verts, 2 tris. Normal points outward.
// --------------------------------------------------------------------------
void CharacterRenderer::AddBox(std::vector<float>& verts,
                               std::vector<uint16_t>& indices,
                               glm::vec3 ctr, glm::vec3 h, glm::vec3 col, int bone)
{
    // 6 face normals, CCW quad vertex offsets, and winding
    struct Face { glm::vec3 n; glm::vec3 v[4]; };
    Face faces[6] = {
        // +X
        { { 1,0,0}, { {h.x,-h.y, h.z},{h.x,-h.y,-h.z},{h.x, h.y,-h.z},{h.x, h.y, h.z} } },
        // -X
        { {-1,0,0}, { {-h.x,-h.y,-h.z},{-h.x,-h.y, h.z},{-h.x, h.y, h.z},{-h.x, h.y,-h.z} } },
        // +Y
        { { 0,1,0}, { {-h.x, h.y, h.z},{h.x, h.y, h.z},{h.x, h.y,-h.z},{-h.x, h.y,-h.z} } },
        // -Y
        { { 0,-1,0}, { {-h.x,-h.y,-h.z},{h.x,-h.y,-h.z},{h.x,-h.y, h.z},{-h.x,-h.y, h.z} } },
        // +Z
        { { 0,0,1}, { {-h.x,-h.y, h.z},{h.x,-h.y, h.z},{h.x, h.y, h.z},{-h.x, h.y, h.z} } },
        // -Z
        { { 0,0,-1}, { {h.x,-h.y,-h.z},{-h.x,-h.y,-h.z},{-h.x, h.y,-h.z},{h.x, h.y,-h.z} } },
    };

    for (auto& f : faces) {
        uint16_t base = (uint16_t)(verts.size() / FLOATS_PER_VERT);
        for (int i = 0; i < 4; ++i) {
            glm::vec3 p = ctr + f.v[i];
            verts.insert(verts.end(), { p.x, p.y, p.z,
                                        f.n.x, f.n.y, f.n.z,
                                        col.x, col.y, col.z,
                                        (float)bone });
        }
        // CCW quad: 0,1,2  0,2,3
        indices.insert(indices.end(), { base, (uint16_t)(base+1), (uint16_t)(base+2),
                                        base, (uint16_t)(base+2), (uint16_t)(base+3) });
    }
}

// --------------------------------------------------------------------------
// BuildMesh — one box per body part, in that part's local space (origin = pivot)
// --------------------------------------------------------------------------
void CharacterRenderer::BuildMesh() {
    std::vector<float>    verts;
    std::vector<uint16_t> indices;

    // Color palette
    glm::vec3 cRed    (1.00f, 0.08f, 0.05f);
    glm::vec3 cGreen  (0.05f, 0.90f, 0.10f);
    glm::vec3 cYellow (1.00f, 0.88f, 0.05f);
    glm::vec3 cOrange (1.00f, 0.45f, 0.05f);

    // ROOT: invisible pivot — no geometry needed; we still have the bone

    // TORSO (BONE_TORSO=1): box from -0.6 to +0.6 wide, -0.8 to +0.8 tall, -0.35 to +0.35 deep
    AddBox(verts, indices, {0,0,0}, {0.60f, 0.80f, 0.35f}, cRed,   BONE_TORSO);

    // HEAD (BONE_HEAD=2): sits above torso pivot
    AddBox(verts, indices, {0,0,0}, {0.38f, 0.38f, 0.38f}, cYellow, BONE_HEAD);

    // L_ARM (BONE_LARM=3): thin slab, pivot at shoulder
    AddBox(verts, indices, {0,-0.45f,0}, {0.18f, 0.45f, 0.18f}, cGreen, BONE_LARM);

    // R_ARM (BONE_RARM=4)
    AddBox(verts, indices, {0,-0.45f,0}, {0.18f, 0.45f, 0.18f}, cGreen, BONE_RARM);

    // L_LEG (BONE_LLEG=5)
    AddBox(verts, indices, {0,-0.55f,0}, {0.20f, 0.55f, 0.20f}, cOrange, BONE_LLEG);

    // R_LEG (BONE_RLEG=6)
    AddBox(verts, indices, {0,-0.55f,0}, {0.20f, 0.55f, 0.20f}, cOrange, BONE_RLEG);

    mIndexCount = (int)indices.size();

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// --------------------------------------------------------------------------
// Update — compute bone world matrices from game state
// --------------------------------------------------------------------------
void CharacterRenderer::Update(glm::vec3 rootPos, float bankAngle,
                               float speed, float stridePhase, float breathPhase)
{
    const float PI = 3.14159265f;

    // Procedural animation parameters
    float lean      = bankAngle * 0.5f;            // lean into turns
    float pitchFwd  = speed * 0.25f;               // forward lean at speed
    float stride    = speed * 0.55f;               // arm/leg swing amplitude
    float breathY   = sinf(breathPhase) * 0.04f;  // idle breathing

    // -- ROOT: world position (character stands on carpet) --
    glm::mat4 root = glm::translate(glm::mat4(1.0f), rootPos);
    // Lean into the banking turn around Z-axis (local)
    root = glm::rotate(root, lean, glm::vec3(0,0,1));
    // Forward pitch
    root = glm::rotate(root, -pitchFwd, glm::vec3(1,0,0));
    mBoneMatrices[BONE_ROOT] = root;

    // -- TORSO: sits at root, add breathing bob --
    glm::mat4 torso = glm::translate(root, glm::vec3(0.0f, 0.85f + breathY, 0.0f));
    mBoneMatrices[BONE_TORSO] = torso;

    // -- HEAD: above torso, slight counter-lean for naturalness --
    glm::mat4 head = glm::translate(torso, glm::vec3(0.0f, 0.80f + 0.38f, 0.0f));
    head = glm::rotate(head, -lean * 0.3f, glm::vec3(0,0,1));
    mBoneMatrices[BONE_HEAD] = head;

    // -- L_ARM: shoulder at torso top-left --
    glm::mat4 lArm = glm::translate(torso, glm::vec3(-0.78f, 0.65f, 0.0f));
    float lArmSwing = sinf(stridePhase) * stride;
    lArm = glm::rotate(lArm, lArmSwing, glm::vec3(1,0,0));
    mBoneMatrices[BONE_LARM] = lArm;

    // -- R_ARM: opposite phase --
    glm::mat4 rArm = glm::translate(torso, glm::vec3( 0.78f, 0.65f, 0.0f));
    float rArmSwing = -sinf(stridePhase) * stride;
    rArm = glm::rotate(rArm, rArmSwing, glm::vec3(1,0,0));
    mBoneMatrices[BONE_RARM] = rArm;

    // -- L_LEG: hip at torso base-left --
    glm::mat4 lLeg = glm::translate(torso, glm::vec3(-0.28f, -0.80f, 0.0f));
    float lLegSwing = -sinf(stridePhase) * stride * 0.9f;
    lLeg = glm::rotate(lLeg, lLegSwing, glm::vec3(1,0,0));
    mBoneMatrices[BONE_LLEG] = lLeg;

    // -- R_LEG: opposite phase --
    glm::mat4 rLeg = glm::translate(torso, glm::vec3( 0.28f, -0.80f, 0.0f));
    float rLegSwing =  sinf(stridePhase) * stride * 0.9f;
    rLeg = glm::rotate(rLeg, rLegSwing, glm::vec3(1,0,0));
    mBoneMatrices[BONE_RLEG] = rLeg;
}

// --------------------------------------------------------------------------
// Render
// --------------------------------------------------------------------------
void CharacterRenderer::Render(glm::mat4 viewProj) {
    if (!mProg || !mVbo || !mIbo) return;

    glUseProgram(mProg);

    // Upload VP
    glUniformMatrix4fv(mLocVP, 1, GL_FALSE, glm::value_ptr(viewProj));

    // Upload all bone matrices
    const char* boneUniNames[BONE_COUNT] = {
        "uBone0","uBone1","uBone2","uBone3","uBone4","uBone5","uBone6"
    };
    for (int i = 0; i < BONE_COUNT; ++i) {
        if (mLocBones[i] >= 0)
            glUniformMatrix4fv(mLocBones[i], 1, GL_FALSE, glm::value_ptr(mBoneMatrices[i]));
    }

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

    int stride = FLOATS_PER_VERT * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(9*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
