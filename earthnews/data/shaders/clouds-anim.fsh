//
//  Shader.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

uniform sampler2D u_diffuseMap;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  #if 0
  
  lowp vec4 colour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  lowp vec4 fix = vec4 (0.8, 0.8, 0.8, 0.55);
  colour *= fix;
  
  #else
  
  lowp vec4 colour = texture2D (u_diffuseMap, v_texcoord);

  lowp float alpha = colour.r;
  colour.rgb *= v_colour.rgb;
  colour.a *= (alpha * v_colour.a);

  #endif
  gl_FragColor = colour;
}
