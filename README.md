## Laser speckle contrast imaging

This project is based on the observation by Lisa Richards et al at the
University of Texas who noted that a medical imaging technique called laser
speckle contrast imaging could be done with inexpensive and commonplace
hardware: specifically a laser pointer and a webcam.

- Richards, L. M., Kazmi, S. M. S., Davis, J. L., Olin, K. E., & Dunn, A. K.
  (2013). Low-cost laser speckle contrast imaging of blood flow using a webcam.
  Biomedical Optics Express, 4(10), 2269–2283.
  http://doi.org/10.1364/BOE.4.002269

The University of Texas team, led by Andrew Dunn, previously provided software
for the interpretation of captured images, but this software has been withdrawn
and was presumably not open source.

**Thus, I aim to develop open source tools to capture and analyse laser speckle
images.**

The University of Texas team used a raw Bayer pattern capture tool provided by
Logitech. This tool has since been withdrawn, and recent Logitech cameras do
not appear to support raw image capture.

However, I have noticed that the Microsoft Kinect provides raw video streams
for its two cameras. It also has an IR laser which (with a minor optical
modification: adding a diffuser) appears to be suitable for illumination.
Kinect devices for the obsolete Xbox 360 can be obtained very cheaply on the
second hand market.

The light absorption spectrum of skin falls steeply at the far end of the
visible spectrum and into the near infrared, thus an IR laser should provide
better images of blood flowing under the skin than visible light.

## Dependencies

- [libtiff](http://www.simplesystems.org/libtiff/)
- [libfreenect](https://github.com/OpenKinect/libfreenect)
- [Boost](http://www.boost.org/)
