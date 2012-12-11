//
//  atmos.vsh
//
//  Created by Ubaka Onyechi on 26/11/2012.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

precision mediump float;

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
