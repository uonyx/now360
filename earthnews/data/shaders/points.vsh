//
//  draw.vsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

uniform mediump mat4 u_projmatrix;
uniform mediump mat4 u_viewmatrix;
attribute mediump vec4 a_position;
attribute lowp vec4 a_colour;
varying lowp vec4 v_colour;

const float spriteWidth = 0.025;
const float screenWidth = 768.0;

void main (void)
{
  v_colour = a_colour;
  
  // convert to view space
  vec4 eyePos = u_viewmatrix * vec4 (a_position.xyz, 1.0);
  
  // scale quad
  vec4 projCorner = u_projmatrix * vec4 (spriteWidth * 0.5, spriteWidth * 0.5, eyePos.z, eyePos.w);
  gl_PointSize = screenWidth * (projCorner.x / projCorner.w);
  //gl_PointSize = 12.0;
  
  gl_Position = u_projmatrix * eyePos;
}

/*
uniform mediump mat4 u_mvpmatrix;
attribute mediump vec4 a_position;
attribute lowp vec4 a_colour;
varying lowp vec4 v_colour;

void main (void)
{
  v_colour = a_colour;
  gl_PointSize = 19.0;
  gl_Position = u_mvpmatrix * a_position;
  //gl_Position = u_mvpmatrix * vec4 (a_position, 0.0, 1.0);
}
*/