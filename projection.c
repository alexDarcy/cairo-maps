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
#define lat_0 20*ratio
#define lon_0 0*ratio
#define nb_lon 20
#define nb_lat 10

#define radius 10
#define width 500
#define height 500
#define x_center width/2
#define y_center height/2
#define nb_points 20
#define scale 20.

#define fill_cell 0

int is_visible(float lat, float lon)
{
  float dist; 
  float prec = radius/1000.; 
  dist = sin(lat_0)*sin(lat*ratio) + cos(lat_0)*cos(lat*ratio)*cos(lon*ratio - lon_0);

  //return (dist > -prec);
  return (dist > 0);
}

/* Input in degrees. Assumes the point is visible */
int map_to_orthographic(float* x, float* y, float lat, float lon)
{
  float lat1 = lat*ratio;
  float lon1 = lon*ratio;
  float dist;
  
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

    /* No need for that anymore */
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

float find_min_visible(float lon1, float lon0, float lat) {
  float dlon, cur;
  int i;

  dlon = (lon1 - lon0) / nb_points;
  for (i = 0; i < nb_points+1; ++i)
  {
    cur = lon0 + i*dlon;
    if (is_visible(lat, cur)) return cur;
  }
}

float find_max_visible(float lon1, float lon0, float lat) {
  float dlon, lon;
  float cur;
  int i;

  dlon = (lon1 - lon0) / nb_points;
  for (i = nb_points; i > -1; --i)
  {
    cur = lon1 - i*dlon;
    if (is_visible(lat, cur)) return cur;
  }
}

/* Replace the segment extremities with the visible extremities */
void set_to_visible(float* first_lon, float* last_lon, float lat) {
  float min_lon, max_lon;
  int x, res;

  if (is_visible(lat, *first_lon)) {
    if (is_visible(lat, *last_lon)) return;
    else {
      //printf("find max %f is visible %d \n", *last_lon, is_visible(*last_lon, lat));
      //printf("lat lon %f %f \n", *last_lon, lat);
      *last_lon = find_max_visible(*first_lon, *last_lon, lat);
    }
  }
  else {
    if (is_visible(lat, *last_lon)) {
      *first_lon = find_min_visible(*first_lon, *last_lon, lat);
    }
    else {
      *first_lon = POINT_UNDEFINED;
      *last_lon = POINT_UNDEFINED;
      return;
    }
  }

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
  //printf("before %f %f \n", first_lon, last_lon);
  set_to_visible(&first_lon, &last_lon, lat);
  //printf("after %f %f \n", first_lon, last_lon);

  first_lon2 = lon;
  last_lon2 = lon+dlon;
  set_to_visible(&first_lon2, &last_lon2, lat - dlat);

  /* All the cell must be visible as we corrected the extremities */
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
  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_arc (cr, x_center, y_center, radius*scale, 0, 2*M_PI);
  cairo_stroke(cr);
}

int main (int argc, char *argv[])
{
  FILE *input;
  cairo_surface_t *cs;
  cairo_t *cr;
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
 // for (i = 3; i < 4; ++i) {
    for (j = 0; j < nb_lon; ++j) {
//    for (j = 0; j < 1; ++j) {
      draw_cell(90.-i*dlat, j*dlon, dlat, dlon, cr);
    }
  }

  draw_outer_circle(cr);

  /* Needed for PDF output */
  cairo_show_page(cr);
  cairo_destroy(cr);

  cairo_surface_flush(cs);
  cairo_surface_destroy(cs);

  return 0;
}

