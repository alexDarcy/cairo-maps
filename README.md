Projection
==========

Plot an orthographic projection of a regular latitude-longitude grid with the
Cairo library.

A first approach is to draw a cell line by line and remove the part of the line
outside the projection.
As the border is missing, we can draw a circle.

However, it does not allow for filling the cells, as Cairo needs a closed path.
This strategy has been implemented, see the following example :

![Small example](https://github.com/alexDarcy/projection/raw/master/fill.png)


A video demonstrating the camera movement can be found 
[there](http://youtu.be/5jUbVnv5__0)
