fname = 'blueeyedhushsort.o71802837'
size1 = 1000000
size2 = 10000000
size3 = 100000000

filter(x,control,threshold)=(control==threshold)?(x):(1/0)

set xlabel "# buckets"
set logscale x 10
set ylabel "# threads"
set zlabel "execution time"

splot fname using 2:3:(filter($8,$1,size1)) with points lc rgb "green" title "N = ".size1, \
      fname using 2:3:(filter($8,$1,size2)) with points lc rgb "blue" title "N = ".size2, \
      fname using 2:3:(filter($8,$1,size3)) with points lc rgb "red" title "N = ".size3

pause -1
