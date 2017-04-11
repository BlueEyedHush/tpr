filter(x,control,threshold)=(control==threshold)?(x):(1/0)
splot 'results.out' using 2:3:(filter($6,$1,10000)) with points lc rgb "green" title "N = 10^5", \
      'results.out' using 2:3:(filter($6,$1,100000)) with points lc rgb "blue" title "N = 10^6", \
      'results.out' using 2:3:(filter($6,$1,1000000)) with points lc rgb "red" title "N = 10^7"
pause -1
