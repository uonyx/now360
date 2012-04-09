//
//  Shader.vsh
//  testGraphics
//
//  Created by Ubaka  Onyechi on 26/12/2011.
//  Copyright (c) 2011 SonOfLagos. All rights reserved.
//

attribute vec4 position;
attribute vec3 normal;
attribute vec2 texCoord;

varying lowp vec4 colorVarying;
varying highp vec2 texCoordVarying;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

const float c_zero = 0.0;

void main()
{
  vec3 eyeNormal = normalize(normalMatrix * normal);
  vec3 lightPosition = vec3(c_zero, c_zero, 1.0);
  vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
  float nDotVP = max(c_zero, dot(eyeNormal, normalize(lightPosition)));
                 
  colorVarying = diffuseColor * nDotVP;
  texCoordVarying = texCoord;
  
  //colorVarying = vec4(1.0, 1.0, 0.0, 1.0);
  gl_Position = modelViewProjectionMatrix * position;
}
