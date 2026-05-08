//
// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

uniform lowp vec3       vMaterialAmbient;
uniform lowp vec4       vMaterialSpecular;

uniform samplerCube samplerObj;   // kept declared so C++ side doesn't error

varying lowp vec4 colorDiffuse;

uniform highp vec3   vLight0;
varying mediump vec3 position;
varying mediump vec3 normal;

void main()
{
    // Specular highlight
    mediump vec3 halfVector = normalize(-vLight0 + position);
    mediump float NdotH    = max(dot(normalize(normal), halfVector), 0.0);
    mediump float specular  = pow(NdotH, vMaterialSpecular.w);
    mediump vec3 specColor  = vMaterialSpecular.xyz * specular * 0.7;

    // Bright R / G / Y palette driven by face normal direction.
    // abs() so back-faces get the same colour as front-faces.
    mediump vec3 n = abs(normalize(normal));
    mediump vec3 palette =
        n.x * vec3(1.0,  0.05, 0.0)   // X-axis faces → vivid red
      + n.y * vec3(0.05, 1.0,  0.05)  // Y-axis faces → vivid green
      + n.z * vec3(1.0,  0.88, 0.0);  // Z-axis faces → vivid yellow
    palette = clamp(palette * 1.3, 0.0, 1.0);

    // Diffuse lighting (keep faces shaded so the shape reads clearly)
    mediump float diffuse = max(dot(normalize(normal), normalize(-vLight0)), 0.0);
    mediump vec3 lit = palette * (0.30 + diffuse * 0.85);

    gl_FragColor = vec4(clamp(lit + specColor, 0.0, 1.0), 1.0);
}
