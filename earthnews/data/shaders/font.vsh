//
//  font.vsh
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

uniform mediump mat4 u_mvpmatrix;

attribute mediump vec2 a_position;
attribute lowp vec4 a_colour;
attribute lowp vec2 a_texcoord;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  v_colour = a_colour;
  v_texcoord = a_texcoord;
  
  gl_Position = u_mvpmatrix * vec4 (a_position, 0.0, 1.0);
}
