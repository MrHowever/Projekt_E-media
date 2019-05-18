set style line 1 \
    linecolor rgb '#0060ad' \
    linetype 1 linewidth 2 \
    pointtype 7 pointsize 1.5

set multiplot layout 2, 1 title "Transformacja DCT"

set xlabel 'N'
set ylabel 'Value'

set xrange [0:1000]
set yrange[0:256]
set pointsize 0.5
set title 'Original data'


plot 'orig_data.dat' title 'Original data' with impulses, 'orig_data.dat' with points pt 7 notitle

set title 'DCT'
set yrange [-200:200]

plot 'dct_data.dat' using 1:2 title 'DCT' with impulses, 'dct_data.dat' with points pt 7 notitle