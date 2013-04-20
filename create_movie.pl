#!/usr/bin/perl

my $k = 0;
my $lat = 0;
my $lon = 0;
my $sign_lat = 1;
for my $k (1..720) {
  print "$lat $lon \n";
  my $file = "step_$k.png";
  system("./projection $lat $lon $file");
  if ($lat >= 90.) { $sign_lat = -$sign_lat};
  if ($lat <= -90.) { $sign_lat = -$sign_lat};
  $lat = $lat + 0.25*$sign_lat;
  if ($lon >= 360.) { $lon = 0.;}
  else { $lon = $lon + 1;}
  $k = $k+1;
}
