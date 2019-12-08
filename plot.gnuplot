#!/usr/bin/gnuplot

!echo 1-core
!./a.out -c 1 -g > 1-core.txt
!tail -1 1-core.txt | awk '{ printf "%s %s 1-core\n", $2, $4 }' > 1-corelabel.txt

!echo 2-core
!./a.out -c 2 -g > 2-core.txt
!tail -1 2-core.txt | awk '{ printf "%s %s 2-core\n", $2, $4 }' > 2-corelabel.txt

!echo 3-core
!./a.out -c 3 -g > 3-core.txt
!tail -1 3-core.txt | awk '{ printf "%s %s 3-core\n", $2, $4 }' > 3-corelabel.txt

!echo 4-core
!./a.out -c 4 -g > 4-core.txt
!tail -1 4-core.txt | awk '{ printf "%s %s 4-core\n", $2, $4 }' > 4-corelabel.txt

!echo 5-core
!./a.out -c 5 -g > 5-core.txt
!tail -1 5-core.txt | awk '{ printf "%s %s 5-core\n", $2, $4 }' > 5-corelabel.txt

!echo 6-core
!./a.out -c 6 -g > 6-core.txt
!tail -1 6-core.txt | awk '{ printf "%s %s 6-core\n", $2, $4 }' > 6-corelabel.txt

!echo 7-core
!./a.out -c 7 -g > 7-core.txt
!tail -1 7-core.txt | awk '{ printf "%s %s 7-core\n", $2, $4 }' > 7-corelabel.txt

!echo 8-core
!./a.out -c 8 -g > 8-core.txt
!tail -1 8-core.txt | awk '{ printf "%s %s 8-core\n", $2, $4 }' > 8-corelabel.txt

!echo 9-core
!./a.out -c 9 -g > 9-core.txt
!tail -1 9-core.txt | awk '{ printf "%s %s 9-core\n", $2, $4 }' > 9-corelabel.txt

!echo 10-core
!./a.out -c 10 -g > 10-core.txt
!tail -1 10-core.txt | awk '{ printf "%s %s 10-core\n", $2, $4 }' > 10-corelabel.txt

!echo 11-core
!./a.out -c 11 -g > 11-core.txt
!tail -1 11-core.txt | awk '{ printf "%s %s 11-core\n", $2, $4 }' > 11-corelabel.txt

!echo 12-core
!./a.out -c 12 -g > 12-core.txt
!tail -1 12-core.txt | awk '{ printf "%s %s 12-core\n", $2, $4 }' > 12-corelabel.txt

!echo 13-core
!./a.out -c 13 -g > 13-core.txt
!tail -1 13-core.txt | awk '{ printf "%s %s 13-core\n", $2, $4 }' > 13-corelabel.txt

!echo 14-core
!./a.out -c 14 -g > 14-core.txt
!tail -1 14-core.txt | awk '{ printf "%s %s 14-core\n", $2, $4 }' > 14-corelabel.txt

!echo 15-core
!./a.out -c 15 -g > 15-core.txt
!tail -1 15-core.txt | awk '{ printf "%s %s 15-core\n", $2, $4 }' > 15-corelabel.txt

!echo 16-core
!./a.out -c 16 -g > 16-core.txt
!tail -1 16-core.txt | awk '{ printf "%s %s 16-core\n", $2, $4 }' > 16-corelabel.txt

set title "Work Stealing Queue (10000 Jobs * Workload Nonoseconds)"
set xlabel "Workload Nanoseconds"
set ylabel "Parallel Speedup"
unset key
set ytics 1
set xtics 500
set grid

plot "1-core.txt" using 2:4 with linespoints, \
     "1-corelabel.txt" with labels offset +4,0, \
     "2-core.txt" using 2:4 with linespoints, \
     "2-corelabel.txt" with labels offset +4,0, \
     "3-core.txt" using 2:4 with linespoints, \
     "3-corelabel.txt" with labels offset +4,0, \
     "4-core.txt" using 2:4 with linespoints, \
     "4-corelabel.txt" with labels offset +4,0, \
     "5-core.txt" using 2:4 with linespoints, \
     "5-corelabel.txt" with labels offset +4,0, \
     "6-core.txt" using 2:4 with linespoints, \
     "6-corelabel.txt" with labels offset +4,0, \
     "7-core.txt" using 2:4 with linespoints, \
     "7-corelabel.txt" with labels offset +4,0, \
     "8-core.txt" using 2:4 with linespoints, \
     "8-corelabel.txt" with labels offset +4,0, \
     "9-core.txt" using 2:4 with linespoints, \
     "9-corelabel.txt" with labels offset +4,0, \
     "10-core.txt" using 2:4 with linespoints, \
     "10-corelabel.txt" with labels offset +4,0, \
     "11-core.txt" using 2:4 with linespoints, \
     "11-corelabel.txt" with labels offset +4,0, \
     "12-core.txt" using 2:4 with linespoints, \
     "12-corelabel.txt" with labels offset +4,0, \
     "13-core.txt" using 2:4 with linespoints, \
     "13-corelabel.txt" with labels offset +4,0, \
     "14-core.txt" using 2:4 with linespoints, \
     "14-corelabel.txt" with labels offset +4,0, \
     "15-core.txt" using 2:4 with linespoints, \
     "15-corelabel.txt" with labels offset +4,0, \
     "16-core.txt" using 2:4 with linespoints, \
     "16-corelabel.txt" with labels offset +4,0
