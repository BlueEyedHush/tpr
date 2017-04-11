fname = 'yoga2_1p.out'
size1 = 10000
size2 = 100000
size3 = 1000000

filter(x,control,threshold)=(control==threshold)?(x):(1/0)

set xlabel "# buckets"
set ylabel "# threads"
set zlabel "execution time"

set term x11 1
splot fname using 2:3:(filter($6,$1,size1)) with points lc rgb "green" title "N = ".size1

set term x11 2
splot fname using 2:3:(filter($6,$1,size2)) with points lc rgb "blue" title "N = ".size2

set term x11 3
splot fname using 2:3:(filter($6,$1,size3)) with points lc rgb "red" title "N = ".size3

set term x11 0
splot fname using 2:3:(filter($6,$1,size1)) with points lc rgb "green" title "N = ".size1, \
      fname using 2:3:(filter($6,$1,size2)) with points lc rgb "blue" title "N = ".size2, \
      fname using 2:3:(filter($6,$1,size3)) with points lc rgb "red" title "N = ".size3

pause -1
