//
//  Shader.fsh
//  testGraphics
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

uniform sampler2D u_sampler;

varying lowp vec4 v_colour;
varying mediump vec2 v_texcoord;

void main (void)
{
  //gl_FragColor = texture2D (u_sampler, v_texcoord) * v_colour;
  gl_FragColor = texture2D (u_sampler, v_texcoord);
}
