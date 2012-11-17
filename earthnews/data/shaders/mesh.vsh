//
//  Shader.vsh
//  testGraphics
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec3 a_tangent;
attribute vec3 a_bitangent;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform mat3 u_normalMatrix;
uniform mediump vec4 u_lightPosition;
uniform mediump vec4 u_eyePosition;

varying lowp vec4 v_colour;
varying mediump vec2 v_texcoord;

const float c_zero = 0.0;
const float c_one = 1.0;

void main (void)
{
  vec3 eyeNormal = normalize (u_normalMatrix * a_normal);
  vec3 lightPosition = vec3 (c_zero, c_zero, -1.5);
  vec4 diffuseColor = vec4 (c_one, c_one, c_one, c_one);
    
  float nDotVP = max(c_zero, dot(eyeNormal, normalize(lightPosition)));
                 
  v_colour = diffuseColor * nDotVP;
  v_texcoord = a_texcoord;

  vec3 use = a_tangent + a_bitangent;
  use.z = 0.0;
  
  gl_Position = u_mvpMatrix * a_position;
}

/*
varying vec3 v_viewDir;
varying vec3 v_lightDir;
void main (void)
{
  
  mat3 tangentMat = mat3 (a_tangent, a_bitangent, a_normal);
}
*/