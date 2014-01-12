//
//  clouds.fsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

uniform sampler2D u_diffuseMap;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  lowp vec4 colour = texture2D (u_diffuseMap, v_texcoord);
  lowp float alpha = colour.r;
  
  colour.rgb *= v_colour.rgb;
  colour.a *= (alpha * v_colour.a);
  
  gl_FragColor = colour;
}
