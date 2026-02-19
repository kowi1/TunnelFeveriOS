#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 4.2.1)


# build native_app_glue as a static lib
# add_library(native_app_glue STATIC
 #    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
set(target_name game)     
set(BULLET_PHYSICS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bullet)
set(FIREBASE_CPP_SDK_DIR /Users/Mac/firebase_cpp_sdk)
# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
#set(CMAKE_SHARED_LINKER_FLAGS
 #   "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")



# Set common compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")
add_definitions("-DGLM_FORCE_SIZE_T_LENGTH -DGLM_FORCE_RADIANS")

# Import the CMakeLists.txt for the glm library
add_subdirectory(glm)


#get_filename_component(ndkHelperSrc ${CMAKE_CURRENT_SOURCE_DIR}/common ABSOLUTE)
#add_subdirectory(${ndkHelperSrc})
#${CMAKE_CURRENT_SOURCE_DIR}/common/ndkHelperBin/${CMAKE_BUILD_TYPE}/${ANDROID_ABI})

# now build app's shared lib
add_library(game STATIC
     anim.cpp
     input_util.cpp
     ascii_to_geom.cpp
     dialog_scene.cpp
     indexbuf.cpp
     native_engine.cpp
     obstacle.cpp
     obstacle_generator.cpp
     our_shader.cpp
     play_scene.cpp
     scene.cpp
     scene_manager.cpp
     shader.cpp
     shape_renderer.cpp
     teapot_renderer.cpp
     tex_quad.cpp
     text_renderer.cpp
     texture_teapot.cpp
     texture.cpp
     textured_teapot_render.cpp
     ui_scene.cpp
     util.cpp
     vertexbuf.cpp
     welcome_scene.cpp
     shaderhelper.cpp
     vecmath.cpp)

target_include_directories(game PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/data
     ${CMAKE_CURRENT_SOURCE_DIR}/glm
     ${CMAKE_CURRENT_SOURCE_DIR}/common/stb)
    # ${BULLET_PHYSICS_SOURCE_DIR}/src
    # ${FIREBASE_CPP_SDK_DIR}/include
     
     

# add lib dependencies
target_link_libraries(game
     #android
     #native_app_glue
     #NdkHelper
     #atomic
     #EGL
     #GLESv2
     glm
    # log
    # OpenSLES
    )


# Add Firebase libraries to the target using the function from the SDK.
add_subdirectory(${FIREBASE_CPP_SDK_DIR} bin/ EXCLUDE_FROM_ALL)

# The Firebase C++ library `firebase_app` is required,
# and it must always be listed last.

# Add the Firebase SDKs for the products you want to use in your app
# For example, to use Analytics, Firebase Authentication, and Firebase Realtime Database
#set(firebase_libs firebase_analytics firebase_auth firebase_gma firebase_database #firebase_functions firebase_storage firebase_messaging firebase_firestore firebase_app_check #firebase_app)
#target_link_libraries(${target_name} "${firebase_libs}")

#add_subdirectory(bullet/src)
#target_link_libraries(${target_name} LinearMath  BulletDynamics BulletCollision BulletSoftBody)

