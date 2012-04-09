//
//  draw.vsh
//

uniform mediump mat4 u_mvpmatrix;

attribute mediump vec2 a_position;
attribute lowp vec4 a_colour;

varying lowp vec4 v_colour;

void main (void)
{
  v_colour = a_colour;
  
  gl_Position = u_mvpmatrix * vec4 (a_position, 0.0, 1.0);
}
