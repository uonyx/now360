//
//  lines.vsh
//
//  Created by Ubaka Onyechi on 04/11/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

uniform mediump mat4 u_mvpmatrix;

attribute mediump vec4 a_position;
attribute lowp vec4 a_colour;

varying lowp vec4 v_colour;

void main (void)
{
  v_colour = a_colour;
  
  gl_Position = u_mvpmatrix * a_position;
}
