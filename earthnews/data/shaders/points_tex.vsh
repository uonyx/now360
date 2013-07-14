//
//  draw.vsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

precision lowp float;

uniform mat4 u_projmatrix;
uniform mat4 u_viewmatrix;
attribute vec4 a_position;
attribute vec4 a_colour;
varying vec4 v_colour;

const float spriteWidth = 0.015; //0.025;
const float screenWidth = 768.0;
const float c_half = 0.5;
const float c_one = 1.0;

void main (void)
{
  v_colour = a_colour;
  
  vec4 eyePos = u_viewmatrix * vec4 (a_position.xyz, c_one);
  vec4 projCorner = u_projmatrix * vec4 (spriteWidth * c_half, spriteWidth * c_half, eyePos.z, eyePos.w);
  
  //gl_PointSize = 12.0;
  gl_PointSize = screenWidth * (projCorner.x / projCorner.w);
  gl_Position = u_projmatrix * eyePos;
}
