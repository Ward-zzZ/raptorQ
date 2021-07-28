set terminal png font "Miso, 16"
set output 'graph.png'


set style fill solid border rgb "#ffffff"
set auto x
set title "RaptorQ benchmark"
set xlabel "Overhead"
set ylabel "Psuc"
set yrange [0:100]
set xrange [1:6]
set xtics nomirror
set ytics 10
set mxtics 5
set grid
plot 'graph_p10.dat' using 1:2 title"Pdrop=10%" with linespoints ls 6,'graph_p30.dat' using 1:2 title"Pdrop=30%" with linespoints ls 6 lt 7
set key right bottom
