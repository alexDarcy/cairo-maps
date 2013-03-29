#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>


#define PI 3.141592653589793238462643383
#define ratio PI/180.
#define POINT_UNDEFINED -99999.

/* Modify these */
#define lat_0 10*ratio
#define lon_0 0*ratio
#define nb_lon 10
#define nb_lat 10

#define radius 10
#define width 500
#define height 500
#define x_center width/2
#define y_center height/2
#define nb_points 20
#define scale 20.

#define fill_cell 0

/* Input in degrees */
int map_to_orthographic(float* x, float* y, float lat, float lon)
{
  float lat1 = lat*ratio;
  float lon1 = lon*ratio;
  float dist;
  float prec = radius/1000.; 
  dist = sin(lat_0)*sin(lat1) + cos(lat_0)*cos(lat1)*cos(lon1 - lon_0);

  /* Only print negative distance, with a precision */
  /* No longer need to check that */
//  if (dist < prec) return -1;

  (*x) = radius*cos(lat1)*sin(lon1 - lon_0);
  (*y) = cos(lat_0)*sin(lat1) - sin(lat_0)*cos(lat1)*cos(lon1 - lon_0);
  (*y) *= radius;

  return 1;
}

/* Draw the visible part of an arc */
void draw_arc(float lat0, float lon0, float lat1, float lon1, cairo_t* cr) 
{
  int i;
  int res1, res2;
  float dlat, dlon;
  float x1, y1;
  float x2, y2;

  dlat = (lat1 - lat0) / nb_points;
  dlon = (lon1 - lon0) / nb_points;
//  printf("dlat dlon %f %f \n", dlat, dlon);
  for (i = 0; i < nb_points; ++i)
  {
    res1 = map_to_orthographic(&x1, &y1, lat0 + i*dlat, lon0 + i*dlon);
    res2 = map_to_orthographic(&x2, &y2, lat0 + (i+1)*dlat, lon0 + (i+1)*dlon);

    if (res1 > 0 && res2 > 0) {
      /* Beware, y=0 is top of the figure, so we must revert */
      if (fill_cell) {
        cairo_line_to(cr, x_center + x2*scale, height-(y_center + y2*scale));
        cairo_stroke_preserve(cr);
      }
      else {
        cairo_move_to(cr, x_center + x1*scale, height-(y_center + y1*scale));
        cairo_line_to(cr, x_center + x2*scale, height-(y_center + y2*scale));
        cairo_stroke(cr);
      }

    }
  }
}

void visible_lon_extrema(float* maxlon1, float* maxlon2, float lat)
{
  /* Lat is in degree */
  *maxlon1 = -sin(lat_0)*sin(lat*ratio)/(cos(lat_0)*cos(lat*ratio));
  *maxlon1 = (acos(*maxlon1) + lon_0)*180./PI;
  *maxlon2 = 360 - *maxlon1;
}

void adapt_lon(float* first_lon, float* last_lon, float lat) {
  float max_lon1, max_lon2;
  int x;

  
  visible_lon_extrema(&max_lon1, &max_lon2, lat);
  if (*first_lon < max_lon1 && *last_lon < max_lon1) 
    x=0;
  else if (*first_lon < max_lon1 && *last_lon > max_lon1) 
    *last_lon = max_lon1;
  else if (*first_lon < max_lon2 && *last_lon < max_lon2)
  {
    *first_lon = POINT_UNDEFINED;
    *last_lon = POINT_UNDEFINED;
    return;
  }
  else if (*first_lon < max_lon2 && *last_lon > max_lon2)
    *first_lon = max_lon2;
}

/* (lat, lon) is upper left corner and in degree */
/* dlat > 0 */
void draw_cell(float lat, float lon, float dlat, float dlon, cairo_t *cr){
  float x1, y1;
  float x2, y2;
  float first_lon, last_lon;
  float first_lon2, last_lon2;
  float r, g, b;

  first_lon = lon;
  last_lon = lon+dlon;
  adapt_lon(&first_lon, &last_lon, lat);

  first_lon2 = lon;
  last_lon2 = lon+dlon;
  adapt_lon(&first_lon2, &last_lon2, lat - dlat);

//  if (first_lon != POINT_UNDEFINED && last_lon != POINT_UNDEFINED) 
//    draw_arc(lat, first_lon, lat, last_lon, cr);
//
//  if (last_lon != POINT_UNDEFINED && last_lon2 != POINT_UNDEFINED) 
//    draw_arc(lat, last_lon, lat - dlat, last_lon2, cr);
//
//  if (last_lon2 != POINT_UNDEFINED && first_lon2 != POINT_UNDEFINED) 
//    draw_arc(lat - dlat, last_lon2, lat - dlat, first_lon2, cr);
//
//  if (first_lon2 != POINT_UNDEFINED && first_lon != POINT_UNDEFINED) 
//    draw_arc(lat - dlat, first_lon2, lat, first_lon, cr);

  if (first_lon2 != POINT_UNDEFINED && first_lon != POINT_UNDEFINED && 
      last_lon2 != POINT_UNDEFINED && last_lon != POINT_UNDEFINED ) 
  {
    cairo_set_source_rgb(cr, 0., 0., 0.);
    draw_arc(lat, first_lon, lat, last_lon, cr);
    draw_arc(lat, last_lon, lat - dlat, last_lon2, cr);
    draw_arc(lat - dlat, last_lon2, lat - dlat, first_lon2, cr);
    draw_arc(lat - dlat, first_lon2, lat, first_lon, cr);
    if (fill_cell) {
      cairo_close_path(cr);
      r = (float)rand()/(float)RAND_MAX;
      g = (float)rand()/(float)RAND_MAX;
      b = (float)rand()/(float)RAND_MAX;
      cairo_set_source_rgb(cr, r, g, b);
      cairo_fill(cr);
    }
  }
}

void draw_outer_circle(cairo_t *cr)
{
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_arc (cr, x_center, y_center, radius*scale, 0, 2*M_PI);
  cairo_stroke(cr);
}

int main (int argc, char *argv[])
{
  FILE *input;
  cairo_surface_t *cs;
  cairo_t *cr;
  /*  int height = 100;
      int width = 100;*/
  float margin = 5.;
  float dlon, dlat;
  float lon;
  int i, j;

  srand(time(NULL));

  /* Create a surface and a cairo_t */
  /* A4 format approx */
  cs = cairo_pdf_surface_create("test.pdf", width, height);
  cr = cairo_create (cs);
  cairo_set_source_rgb(cr,0,0,0);
  cairo_set_line_width(cr,0.5);
  cairo_stroke(cr);

  dlon = 360./nb_lon;
  dlat = 180./nb_lat;
  for (i = 0; i < nb_lat; ++i) {
  //for (i = 3; i < 4; ++i) {
    //for (j = 0; j < 1; ++j) {
    for (j = 0; j < nb_lon; ++j) {
      draw_cell(90.-i*dlat, j*dlon, dlat, dlon, cr);
    }
  }

  //draw_outer_circle(cr);

  /* Needed for PDF output */
  cairo_show_page(cr);
  cairo_destroy(cr);

  cairo_surface_flush(cs);
  cairo_surface_destroy(cs);

  return 0;
}

