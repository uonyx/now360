Nov, 2013

![Now360](http://a3.mzstatic.com/eu/r30/Purple4/v4/71/72/af/7172af6b-5c6a-fd21-3ca0-3e00d7a31d45/screen480x480.jpeg)

## Background
Done purely for educational reasons to re-familiarise myself with graphics programming, 3D maths, C, and to improve knowledge of the iOS platform. Being an iOS project, Objective-C was unavoidable and some of its use in this project is very questionable but I learn ;)
- Written in C (with the necessary Objective-C) in my spare time.
- Inspired by / Ode to [Life with Playstation](http://www.playstation.com/life). Perfect choice for an app as it doesn't require use of 3D modelling packages where I have no skill.
- Test app for cx_engine framework (which was written in parallel).
- Started just before the release of iOS6 (I have no idea what the app looks like in iOS7 yet) and released on the app store in August 2013. 

## Implementation
### Set-up (ViewController.h/m, app.h/c, earth.json)
- Xcode OpenGL project - uses GLKit for set up of graphics contexts
- Storyboard used to set up the view. The rest of the app's UI was implemented programmatically.
- City names, weather id, timezone information, and location were painfully and manually pulled from the internet. (There's are better ways to do this programmatically! On iOS, see the Core Location API).

### Earth (earth.h/c)
#### Composition
The texture images (rectangular maps of the earth) were downloaded from NASA's [Visible Earth](http://visibleearth.nasa.gov) project and had colour modifications applied to them. The earth's visual is composed of 3 layered spheres. 
- Topography (inner sphere - topo-hi.vsh/fsh)
  * 12 day-time maps, 1 for each month of the year.
  * 1 night-time map
  * 1 specular light map to add extra shininess on the water area of the map. 
  * 1 normal map for a bump effect on the terrain
- Clouds (middle sphere - clouds-anim.vsh/fsh)
  * 1 cloud map visible in day time
  * Slowly rotates around the y-axis to give pseudo-live planet effect
- Atmospheric glow (outer sphere - atmos.vsh/fsh)
  * Rim light effect shader

##### Sphere Tessellation
Each sphere is tessellated using the common latitude-longitude grid approach; suitable for equirectangular projection of the map images. This mapping makes it very easy to compute texture coordinates which in turn is the solution for finding the world space positions of each city given their longitude and latitude. 

#### Computing the Sun's position 
Wikipedia article on calculating the [position of the sun](http://en.wikipedia.org/wiki/Position_of_the_Sun) - contains useful and educational external links.

### Camera (camera.h/c)
The camera, as expected, manages the user's view of the earth and is tightly linked to the user's input. The way it works is that as the user swipes the screen from left to right, the camera orbits the earth from right to left thereby giving the illusion of the earth spinning in the same direction as the finger's movement. The faster the swipe, the faster the acceleration of the earth's spin. You'll notice the earth also has some deceleration built into it's movement. This deceleration is also present even when dragging. Implementing dragging without deceleration just made the earth feel like a plastic ball. The deceleration gives off a lag effect that gives the impression that you're controlling a huge sphere mass that our planet truly is.

### Feeds (feeds.h/m)
As is displayed during the loading phase of the app:
- Weather information comes from [Yahoo](http://developer.yahoo.com/weather/)
- Tweet streams are from [Twitter](https://dev.twitter.com/docs/api/1.1)
- News headlines and content are from [Google News RSS feeds](https://support.google.com/news/topic/2428792?hl=en-GB&ref_topic=2428789)
These services are all HTTP-based and instructions on a client implementation, along with rules and restrictions, can be found on their websites. (Saying that, there seems to be little or no documentation about Google News now - dammit google!).

### Webview (webview.h/m)
This went through 3 iterations. 
- The first attempt was to use a webview but with OpenGL UI for view navigation etc by "cleverly" rendering the UI in the right places. Worked but had problems with update timings of the UI view and the GL view. 
- The second attempt was to use SVWebViewController which was great but then I didn't particularly like the button placements and wanted to replace the UIActonSheet-functionality with the newer UIActivityViewController; and hacking the existing code to do what I wanted was turning into a mess. 
- So on the third attempt, inspired by SVWebViewController, I wrote my own webview implementation from scratch to look and function exactly how I wanted.

### OSD (ui.h/c, ui_ctlr.h/c)
A mess. I initially had a grand plan of writing this sweet mother of all UI solutions and then halfway through realised I was just over-engineering stuff and then hacked pieces of OSD for the twitter feeds, music player, clock display and other buttons. It's still a mess. 

### Music (audio.h/m)
Sound effects are played using Apple's System Sound Services API which is a very straight-forward and easy to use API. For playing music tracks, the MPMediaPickerController and MPMusicPlayerController APIs make it easy to access and play back music from the user's iPod playlist. The only problem I had and which I couldn't solve was overriding the default action of stopping a playing track when the app was backgrounded. Getting the app to 'pause' instead when backgrounded (then resuming from pause state when foregrounded again) was impossible with these APIs.

### Other stuff
- Added a very rudimentary profanity filter for the kids.
- Timezone information accounts for Daylight Savings Time (DST).
- Originally targeted 60fps but settled for 30fps. (iPad 4s did about 50fps).

## Code and License
The code is very much the same as when the app was released. Some good, some bad, and some downright ugly :)

Copyright [2013] [Ubaka Onyechi]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
            WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
            See the License for the specific language governing permissions and
            limitations under the License.

