//
//  font.fsh
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

uniform sampler2D u_sampler;

varying vec4 v_colour;
varying vec2 v_texcoord;

void main (void)
{
  vec4 colour = texture2D (u_sampler, v_texcoord) * v_colour;
  
  gl_FragColor = colour;
}
