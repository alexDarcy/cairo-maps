#!/usr/bin/perl

my $k = 0;
my $lat = 0;
for (my $lon=0; $lon <= 360; $lon=$lon+1){
  my $file = "step_$k.pdf";
  my $ffile = "step_$k.png";
  system("./projection $lat 0. $file");
  system("convert $file $ffile");
  $lat = $lat + 0.2;
  $k = $k+1;
}
