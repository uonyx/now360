//
//  font.fsh
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

precision lowp float;

uniform sampler2D u_sampler;

varying vec4 v_colour;
varying vec2 v_texcoord;

void main (void)
{
  vec4 colour = texture2D (u_sampler, v_texcoord);
  
  colour.rgb = v_colour.rgb;
  colour.a *= v_colour.a;
  
  gl_FragColor = colour;
}
