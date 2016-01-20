Tools for Image and Volume Visualization
========================================

These are very simple tools to show images and volumes from a variety of formats.

Getting Submodules
------------------

  ```
  git submodule update --init
  ```

Dependencies
------------

  * CMake, Git, Mercurial, GCC
  * libboost-all-dev (make sure libboost-timer-dev and libboost-python-dev are included)
  * liblapack-dev
  * libfftw3-dev
  * libx11-dev
  * libx11-xcb-dev
  * libxcb1-dev
  * libxrandr-dev
  * libxi-dev
  * freeglut3-dev
  * libglew1.6-dev
  * libpng12-dev
  * libtiff4-dev
  * libhdf5-serial-dev
  * libfreetype6-dev
  * ftgl-dev

  On Ubuntu, you can get these packages via

  ```
  sudo apt-get install libboost-all-dev liblapack-dev libfftw3-dev libx11-dev libx11-xcb-dev libxcb1-dev libxrandr-dev libxi-dev freeglut3-dev libglew1.6-dev libpng12-dev libtiff4-dev libhdf5-serial-dev libfreetype6-dev ftgl-dev
  ```

Configure:
----------

  ```
  mkdir build
  cd build
  cmake ..
  ```

Compile:
--------

  ```
  make
  ```
Usage:
------

### Image Viewer

  ```
  image_viewer <path_to_images>
  ```

  The path can be a single image or a directory containing images. In the
  viewer, you can cycle through the images using `a` and `d`. You can zoom and
  pan using `Ctrl` and the mouse whell and dragging.

### Volume Viewer

  ```
  volume_viewer <path_to_volume>
  ```

  The path to the volume can be a directory with a sequence of image files, or a string of the form:

  ```
  <hdf_file:path_to_dataset>
  ```

  The dataset is expected to be a 3D volume. It may contain additional
  attributes `resolution` and `offset`, which are expected to be a vector of
  three floating point values.

  You can show an overlay (e.g., segment ids) using the `--overlay
  <path_to_volume>` option. The overlay will be shown transparently.
  Double-clicking on a segment will show a marching cubes visualization.
