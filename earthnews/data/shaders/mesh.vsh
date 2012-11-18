//
//  Shader.vsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec3 a_tangent;
attribute vec3 a_bitangent;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform mat3 u_normalMatrix;

varying vec4 v_colour;
varying vec2 v_texcoord;

const float c_zero = 0.0;
const float c_one = 1.0;

void main (void)
{
  vec3 use = a_tangent + a_bitangent;
  
  vec3 eyeNormal = normalize (u_normalMatrix * a_normal);
  vec3 lightDirection = vec3 (c_zero, c_zero, -2.0); // target - position
  vec4 diffuseColor = vec4 (c_one, c_one, c_one, c_one);
    
  float nDotVP = max(c_zero, dot(eyeNormal, normalize(lightDirection)));
                 
  v_colour = diffuseColor * nDotVP;
  v_texcoord = a_texcoord;
  
  gl_Position = u_mvpMatrix * a_position;
}
