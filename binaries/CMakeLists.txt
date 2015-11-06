define_module(image_viewer    BINARY SOURCES image_viewer.cpp    LINKS imageprocessing gui io)
define_module(volume_viewer   BINARY SOURCES volume_viewer.cpp   LINKS imageprocessing gui io)
define_module(skeleton_viewer BINARY SOURCES skeleton_viewer.cpp LINKS imageprocessing gui io)
