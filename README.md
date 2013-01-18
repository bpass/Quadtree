Quadtree
========

The purpose of this program is to examine the efficiency of using a Quadtree structure with raster map images.  Currently the only functioning file type is Erdas Imagine images.

To run the program simply use 'make all', which will create an executable called 'quadtree'.  By default the image being used is the nlcd_2006.img image of Colorado.  This can be changed in the source of main.cpp.  

When the executable is run, the results will be created in a new folder called 'Results'.  Currently the output includes test files that represent the quadtree, as well as the reclassified images created.
