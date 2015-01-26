# Color Sparrow Fork

This is a fork of the "Color Sparrow" color constancy algorithm as described in [1] and available at [http://www.fer.unizg.hr/ipg/resources/color_constancy](http://www.fer.unizg.hr/ipg/resources/color_constancy).

The original README can be found below.

## Original README

Color Sparrow is a simple auto white balancing program for removing the color cast of the scene illumination source from a given image. It assumes uniform scene illumination and performs global illumination estimation and after that chromatically adapts the image. The illumination estimation is based on the paper mentioned in the Literature section.

The program needs OpenCV library for image reading and simple processing. For compilation on Linux, the program also needs pkg-config.

### Version

This is version 0.1, the first version of Color Sparrow. Bug reports and feedback are welcome.

### Download

You can always find the newest version at [http://www.fer.unizg.hr/ipg/resources/color_constancy/](http://www.fer.unizg.hr/ipg/resources/color_constancy/).

### License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).

### Literature

    [1] N. Banic and S. Loncaric.
        Using the Random Sprays Retinex algorithm for global illumination estimation.
        Croatian Computer Vision Workshop, 2013, pp. 3–8.

Please send any comments, suggestions or bug fixes to Nikola Banic <nikola.banic@fer.hr>

## License

For license information see above.

For license information of `Lenna.png` see [http://en.wikipedia.org/wiki/Lenna#mediaviewer/File:Lenna.png](http://en.wikipedia.org/wiki/Lenna#mediaviewer/File:Lenna.png).
