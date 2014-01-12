//
//  points_tex.vsh
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

uniform mat4 u_projmatrix;
uniform mat4 u_viewmatrix;
uniform float u_sw;
uniform float u_pw;

attribute vec4 a_position;
attribute vec4 a_colour;

varying vec4 v_colour;

const float c_baseSpriteWidth = 0.01;
const float c_half = 0.5;
const float c_one = 1.0;

void main (void)
{
  v_colour = a_colour;
  
  float spriteWidth = c_baseSpriteWidth * u_pw;
  
  // convert to view space
  vec4 eyePos = u_viewmatrix * vec4 (a_position.xyz, c_one);
  
  // determine point scale based on projection
  vec4 projCorner = u_projmatrix * vec4 (spriteWidth * c_half, spriteWidth * c_half, eyePos.z, eyePos.w);
  
  // set size and position
  gl_PointSize = u_sw * (projCorner.x / projCorner.w);
  gl_Position = u_projmatrix * eyePos;
}
