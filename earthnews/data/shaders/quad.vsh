//
//  draw.vsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

uniform mediump mat4 u_mvpmatrix;
uniform mediump float u_z;

attribute mediump vec2 a_position;
attribute lowp vec4 a_colour;

varying lowp vec4 v_colour;

void main (void)
{
  v_colour = a_colour;
  
  gl_Position = u_mvpmatrix * vec4 (a_position, u_z, 1.0);
}
