set palette gray
unset xlabel
unset ylabel
unset xtics
unset ytics
unset border
set title ' '
set xrange [1:1664]
set yrange [1:280]
infile='cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-27-0__4_0.asc'
plot infile matrix with image
