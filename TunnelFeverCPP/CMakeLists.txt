cmake_minimum_required(VERSION 3.19)
project(TunnelFeverCPP CXX C)

# ---------------------------------------------------------------------------
# Paths (all relative so the project is portable on this machine)
# ---------------------------------------------------------------------------
set(ROOT_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/..")       # TunnelFeveriOS/
set(CPP_DIR        "${ROOT_DIR}/cpp")                       # TunnelFeveriOS/cpp/
set(BULLET_DIR     "${CPP_DIR}/bullet")                      # Bullet root (has src/ inside)
set(BULLET_SRC_DIR "${BULLET_DIR}/src")                      # Bullet source root
set(FIREBASE_INCLUDE_DIR "/Users/mac/firebase_cpp_sdk/include")
# Stub headers for Firebase GMA — the C++ engine includes these but never calls
# GMA API directly on iOS (ads go through the Swift layer).
set(FIREBASE_STUBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stubs")

# ---------------------------------------------------------------------------
# Global C++ settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(
    -DGLM_FORCE_SIZE_T_LENGTH
    -DGLM_FORCE_RADIANS
)

# ---------------------------------------------------------------------------
# Bullet build knobs — disable everything we don't need
# ---------------------------------------------------------------------------
set(BUILD_BULLET3                   OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS             OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS             OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS                 OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS                    OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS                OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS               OFF CACHE BOOL "" FORCE)
set(USE_GLUT                        OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK         OFF CACHE BOOL "" FORCE)
set(USE_DOUBLE_PRECISION            OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS                    OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_FILES             OFF CACHE BOOL "" FORCE)
# Bullet's own CMakeLists.txt appends /src to this variable, so point to the
# directory that CONTAINS src/ (i.e. the bullet/ root, not bullet/src/).
set(BULLET_PHYSICS_SOURCE_DIR "${BULLET_DIR}" CACHE PATH "" FORCE)

add_subdirectory("${BULLET_SRC_DIR}" BulletBuild EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# TunnelFeverCPP — the game engine as a static library
# ---------------------------------------------------------------------------
add_library(TunnelFeverCPP STATIC
    "${CPP_DIR}/anim.cpp"
    "${CPP_DIR}/ascii_to_geom.cpp"
    "${CPP_DIR}/asset_util.cpp"
    "${CPP_DIR}/dialog_scene.cpp"
    "${CPP_DIR}/indexbuf.cpp"
    "${CPP_DIR}/input_util.cpp"
    # jni_util.cpp is Android-only (JNI types not present on iOS)
    "${CPP_DIR}/native_engine.cpp"
    "${CPP_DIR}/obstacle.cpp"
    "${CPP_DIR}/obstacle_generator.cpp"
    "${CPP_DIR}/play_scene.cpp"
    "${CPP_DIR}/scene.cpp"
    "${CPP_DIR}/scene_manager.cpp"
    "${CPP_DIR}/our_shader.cpp"
    "${CPP_DIR}/shader.cpp"
    "${CPP_DIR}/shape_renderer.cpp"
    "${CPP_DIR}/teapot_renderer.cpp"
    "${CPP_DIR}/tex_quad.cpp"
    "${CPP_DIR}/text_renderer.cpp"
    "${CPP_DIR}/texture.cpp"
    "${CPP_DIR}/texture_teapot.cpp"
    "${CPP_DIR}/textured_teapot_render.cpp"
    "${CPP_DIR}/ui_scene.cpp"
    "${CPP_DIR}/util.cpp"
    "${CPP_DIR}/vecmath.cpp"
    "${CPP_DIR}/vertexbuf.cpp"
    "${CPP_DIR}/welcome_scene.cpp"
    "${CPP_DIR}/HelloWorld.cpp"
    "${CPP_DIR}/shaderhelper.cpp"
    "${CPP_DIR}/sfxman.cpp"
)

target_include_directories(TunnelFeverCPP PRIVATE
    "${FIREBASE_STUBS_DIR}"     # must come first to shadow missing GMA headers
    "${CPP_DIR}"
    "${ROOT_DIR}"
    "${CPP_DIR}/data"
    "${CPP_DIR}/common/stb"
    "${BULLET_SRC_DIR}"
    "${FIREBASE_INCLUDE_DIR}"
)

# Force-include the ObjC compatibility header so objc_object etc. are declared
# when these files are compiled as plain C++ (not Objective-C++).
target_compile_options(TunnelFeverCPP PRIVATE
    -include "${CMAKE_CURRENT_SOURCE_DIR}/stubs/apple_compat.h"
)

target_link_libraries(TunnelFeverCPP PRIVATE
    BulletDynamics
    BulletCollision
    LinearMath
)

# Expose Bullet headers to any consumer that links TunnelFeverCPP
target_include_directories(TunnelFeverCPP PUBLIC
    "${BULLET_SRC_DIR}"
)
