//
//  draw.fsh
//

varying lowp vec4 v_colour;

void main (void)
{
  gl_FragColor = v_colour;
}
