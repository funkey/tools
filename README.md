Tools for Image and Volume Visualization
========================================

These are very simple tools to show images, volumes, and skeletons from a variety of formats.

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

  On Ubuntu 14.04, you can get these packages via

  ```
  sudo apt-get install libboost-all-dev liblapack-dev libfftw3-dev libx11-dev libx11-xcb-dev libxcb1-dev libxrandr-dev libxi-dev freeglut3-dev libglew1.6-dev libpng12-dev libtiff4-dev libhdf5-serial-dev libfreetype6-dev ftgl-dev libfontconfig1-dev
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
  
  Skeletons can be visualized with the `--skeleton` command line option.
  The given file should be in the ITK graph format.

#### Keyboard Controls

  * `s` show skeleton nodes as spheres
  * `l` toggle display of labels
  * `r` reset transformations
  * `Tab` change opacity of volume renderings (opaque, translucent, invisible)

#### Mouse Controls

  * left click and drag: rotate view
  * wheel: step through stack
  * left double click: render label under cursor as volume (can take a while before it shows)
  * `Ctrl` + left click and drag: pan
  * `Ctrl` + wheel: zoom
  * `Shift` + wheel: increase/decrease diameter of skeleton nodes
  
### Image Viewer

  ```
  image_viewer <path_to_images>
  ```

  The path can be a single image or a directory containing images. In the
  viewer, you can cycle through the images using `a` and `d`. You can zoom and
  pan using `Ctrl` and the mouse whell and dragging.
