//
//  draw.fsh
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

varying vec4 v_colour;

void main (void)
{
  gl_FragColor = v_colour;
}
