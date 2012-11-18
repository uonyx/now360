//
//  Shader.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

uniform sampler2D u_diffuseMap;
uniform sampler2D u_nightMap;

varying lowp vec4 v_colour;
varying mediump vec2 v_texcoord;


void main (void)
{
  lowp vec4 white = vec4 (1.0, 1.0, 1.0, 1.0);
  
  lowp vec4 diffuseColour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  lowp vec4 nightColour = texture2D (u_nightMap, v_texcoord) * (white - v_colour);
  
  gl_FragColor = diffuseColour + nightColour;

  //gl_FragColor = texture2D (u_diffuseMap, v_texcoord);
}