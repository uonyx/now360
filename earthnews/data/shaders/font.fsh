//
//  font.fsh
//

uniform sampler2D u_sampler;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  lowp vec4 colour = texture2D (u_sampler, v_texcoord);
  
  colour.r = v_colour.r;
  colour.g = v_colour.g;
  colour.b = v_colour.b;
  colour.a *= v_colour.a;
  
  gl_FragColor = colour;
}
