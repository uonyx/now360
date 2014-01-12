//
//  atmos.vsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

attribute vec4 a_position;
attribute vec3 a_normal;

uniform mat4 u_mvpMatrix;

varying vec3 v_normal;
varying vec4 v_position;

void main (void)
{
  v_normal = a_normal;
  v_position = a_position;
  
  gl_Position = u_mvpMatrix * a_position;
}
