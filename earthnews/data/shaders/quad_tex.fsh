//
//  font.fsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

uniform sampler2D u_sampler;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  lowp vec4 colour = texture2D (u_sampler, v_texcoord);
  
  gl_FragColor = colour * v_colour;
}
