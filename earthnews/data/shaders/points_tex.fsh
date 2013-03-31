//
//  draw.fsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

precision lowp float;

uniform sampler2D u_sampler;
varying vec4 v_colour;

void main (void)
{
  gl_FragColor = texture2D (u_sampler, gl_PointCoord) * v_colour;
}
