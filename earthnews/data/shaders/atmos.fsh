//
//  atmos.fsh
//
//  Created by Ubaka Onyechi on 26/11/2012.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

precision mediump float;

varying vec3 v_normal;
varying vec4 v_position;

uniform vec4 u_lightPosition;
uniform vec4 u_eyePosition;

void main (void)
{
  vec3 nVec = normalize (v_normal);
  vec3 lVec = normalize ((u_lightPosition - v_position).xyz);
  vec3 vVec = normalize ((u_eyePosition - v_position).xyz);
  
  vec4 diffuseColor = vec4 (0.551, 0.702, 0.838, 1.0);

  float rim = 1.0 - max (dot (nVec, vVec), 0.0);
  float rimpower = 6.0; // the higher the value, the thinner the rim
  
  vec4 colour = diffuseColor * pow (rim, rimpower);
  
  float dif = max (dot (nVec, lVec), 0.0);
  dif = smoothstep (0.01, 0.3, dif);
  
  colour.rgb *= dif;
  colour.a = rim;
  
  gl_FragColor = colour;
}
