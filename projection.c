#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793238462643383
#define ratio PI/180.

/* Modify these */
#define lat_0 10*ratio
#define lon_0 0*ratio
#define nb_lon 20
#define nb_lat 10

#define radius 10
#define width 500
#define height 500
#define x_center width/2
#define y_center height/2
#define nb_points 30
#define scale 20.

/* Input in degrees */
int map_to_orthographic(float* x, float* y, float lat, float lon)
{
  float lat1 = lat*ratio;
  float lon1 = lon*ratio;
  float dist;
  float prec = radius/1000.; 
  dist = sin(lat_0)*sin(lat1) + cos(lat_0)*cos(lat1)*cos(lon1 - lon_0);

  /* Only print negative distance, with a precision */
  if (dist > prec) return -1;

  (*x) = radius*cos(lat1)*sin(lon1 - lon_0);
  (*y) = cos(lat_0)*sin(lat1) - sin(lat_0)*cos(lat1)*cos(lon1 - lon_0);
  (*y) *= radius;

  return 1;
}

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
      cairo_move_to(cr, x_center + x1*scale, y_center + y1*scale);
      cairo_line_to(cr, x_center + x2*scale, y_center + y2*scale);
      cairo_stroke(cr);
    }
  }
}

/* (lat, lon) is upper left corner */
/* dlat > 0 */
void draw_cell(float lat, float lon, float dlat, float dlon, cairo_t *cr){
  float x1, y1;
  float x2, y2;

  draw_arc(lat, lon, lat, lon + dlon, cr);
  draw_arc(lat, lon + dlon, lat - dlat, lon + dlon, cr);
  draw_arc(lat - dlat, lon + dlon, lat - dlat, lon, cr);
  draw_arc(lat - dlat, lon, lat, lon, cr);
  
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
  //for (i = nb_lat/2; i < nb_lat/2+1; ++i) {
//    printf("lat latnext %f %f \n", 90.-i*dlat, dlat);
    for (j = 0; j < nb_lon; ++j) {
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

