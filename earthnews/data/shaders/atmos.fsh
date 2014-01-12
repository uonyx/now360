//
//  atmos.fsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
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
  vec4 diffuseColor = vec4 (0.33, 0.80, 0.99, c_one); // my blue

  float dotp = dot (nVec, vVec); // view dot

  float rim = c_one - max (dotp, c_zero);

  //vec4 colour = diffuseColor  * pow (rim, c_rimpower);
  vec4 colour = diffuseColor * (rim * rim * rim);
  
  float dif = max (dot (nVec, lVec), c_zero);
  //dif = smoothstep (0.01, 0.3, dif);
  //dif = clamp ((dif - 0.01) / 0.29, c_zero, c_one);
  dif = (dif - 0.01) / 0.29;
  dif = min (max (dif, c_zero), c_one);
  
  colour.rgb *= dif;
  //colour.a = rim;
  
  gl_FragColor = colour;
}
