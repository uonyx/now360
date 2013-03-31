//
//  atmos.fsh
//
//  Created by Ubaka Onyechi on 26/11/2012.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

precision lowp float;

varying vec3 v_normal;
varying vec4 v_position;

uniform vec4 u_lightPosition;
uniform vec4 u_eyePosition;

const float c_zero = 0.0;
const float c_one = 1.0;
const float c_rimpower = 3.0; // the higher the value, the thinner the rim

void main (void)
{
  vec3 nVec = normalize (v_normal);
  vec3 lVec = normalize ((u_lightPosition - v_position).xyz);
  vec3 vVec = normalize ((u_eyePosition - v_position).xyz);
  
  vec4 diffuseColor = vec4 (0.551, 0.702, 0.838, c_one);

  float dotp = dot (nVec, vVec); // view dot

  #if 1
  
  float rim = c_one - max (dotp, c_zero);
  
  vec4 colour = diffuseColor * rim * rim;// * pow (rim, c_rimpower);
  //vec4 colour = diffuseColor  * pow (rim, c_rimpower);
  
  float dif = max (dot (nVec, lVec), c_zero);
  //dif = smoothstep (0.01, 0.3, dif);
  dif = clamp ((dif - 0.01) / 0.29, c_zero, c_one);
  
  colour.rgb *= dif;
  //colour.a = rim;
  
  #else
  
  vec4 colour = vec4 (c_zero);
  
  if (dotp > c_zero)
  {
    float rim = c_one - dotp;

    colour = diffuseColor * pow (rim, c_rimpower);
    
    float dif = max (dot (nVec, lVec), c_zero);
    dif = clamp ((dif - 0.01) / 0.29, c_zero, c_one);
    
    colour.rgb *= dif;
    colour.a = rim;
  }
  
  #endif
  
  gl_FragColor = colour;
}
