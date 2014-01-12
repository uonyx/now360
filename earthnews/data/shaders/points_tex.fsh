//
//  points_tex.fsh
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

uniform sampler2D u_sampler;
varying vec4 v_colour;

void main (void)
{
  gl_FragColor = texture2D (u_sampler, gl_PointCoord) * v_colour;
}
