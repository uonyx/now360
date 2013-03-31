//
//  draw.fsh
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

precision lowp float;

varying vec4 v_colour;

void main (void)
{
  gl_FragColor = v_colour;
}
