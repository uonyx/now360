//
//  draw.vsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

precision lowp float;

uniform mat4 u_mvpmatrix;
uniform float u_z;

attribute vec2 a_position;
attribute vec4 a_colour;

varying vec4 v_colour;

void main (void)
{
  v_colour = a_colour;
  
  gl_Position = u_mvpmatrix * vec4 (a_position, u_z, 1.0);
}
