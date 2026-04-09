set size ratio 4.17
set terminal pngcairo
set output 'mars001.png'
unset xlabel
unset ylabel
unset xtics
unset ytics
unset border
set title ' '
set xrange [1:768]
set yrange [1:3200]
set palette gray
plot 'm1501995.asc' matrix with image
