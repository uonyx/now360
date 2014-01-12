//
//  topo-hi.vsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec3 a_tangent;
attribute vec3 a_bitangent;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform vec4 u_eyePosition;
uniform vec4 u_lightPosition;

varying vec2 v_texcoord;
varying vec3 v_viewDir;
varying vec3 v_lightDir;

void main (void)
{
  // view and light direction vectors
  vec3 vVec = (u_eyePosition - a_position).xyz;
  vec3 lVec = (u_lightPosition - a_position).xyz;
  
  // tangent matrix
  mat3 tangentMat = mat3 (a_tangent, a_bitangent, a_normal);
  
  // transform view and light vectors to tangent space
  
  v_viewDir = vVec * tangentMat;
  v_lightDir = lVec * tangentMat;
  
  // tex coords
  v_texcoord = a_texcoord;
  
  // vertex position
  gl_Position = u_mvpMatrix * a_position;
}
