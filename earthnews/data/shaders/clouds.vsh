//
//  clouds.vsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform vec4 u_lightPosition;

varying vec4 v_colour;
varying vec2 v_texcoord;

const float c_zero = 0.0;
const float c_one = 1.0;

void main (void)
{
  vec3 eyeNormal = normalize (a_normal);
  vec3 lightDirection = (u_lightPosition - a_position).xyz;
  vec4 diffuseColor = vec4 (c_one, c_one, c_one, c_one);
  
  float nDotVP = max (c_zero, dot(eyeNormal, normalize (lightDirection)));
  
  v_colour = diffuseColor * nDotVP;
  
  v_texcoord = a_texcoord;
  
  gl_Position = u_mvpMatrix * a_position;
}
