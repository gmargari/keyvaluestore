title_imm='Remerge'
title_rng='Rangemerge'
title_geo='Geometric r=3'
title_gp2='Geometric p=2'
title_cas='SMA k=4'
title_cl2='SMA k=2'
title_log='Geometric r=2'
title_nom='Nomerge'

color_imm="'#FF8000'"
color_rng="'#E4191C'"
color_geo="'#4CB04A'"
color_gp2="'#994FA4'"
color_cas="'#387DB8'"
color_cl2="'#000000'"
color_log="'#A65728'"
color_nom="'#888888'"

pointtype_imm="2"
pointtype_rng="1"
pointtype_geo="3"
pointtype_gp2="4"
pointtype_cas="5"
pointtype_cl2="6"
pointtype_log="7"
pointtype_nom="8"

linewidth="lw 3"
style_imm="linespoints $linewidth lt 1 pt $pointtype_imm lc rgb $color_imm"
style_rng="linespoints $linewidth lt 2 pt $pointtype_rng lc rgb $color_rng"
style_geo="linespoints $linewidth lt 3 pt $pointtype_geo lc rgb $color_geo"
style_gp2="linespoints $linewidth lt 4 pt $pointtype_gp2 lc rgb $color_gp2"
style_cas="linespoints $linewidth lt 5 pt $pointtype_cas lc rgb $color_cas"
style_cl2="linespoints $linewidth lt 6 pt $pointtype_cl2 lc rgb $color_cl2"
style_log="linespoints $linewidth lt 7 pt $pointtype_log lc rgb $color_log"
style_nom="linespoints $linewidth lt 8 pt $pointtype_nom lc rgb $color_nom"

linewidth="lw 3"
lines_imm="lines $linewidth lt 1 lc rgb $color_imm"
lines_rng="lines $linewidth lt 2 lc rgb $color_rng"
lines_geo="lines $linewidth lt 3 lc rgb $color_geo"
lines_gp2="lines $linewidth lt 4 lc rgb $color_gp2"
lines_cas="lines $linewidth lt 5 lc rgb $color_cas"
lines_cl2="lines $linewidth lt 6 lc rgb $color_cl2"
lines_log="lines $linewidth lt 7 lc rgb $color_log"
lines_nom="lines $linewidth lt 8 lc rgb $color_nom"

color="#387DB8"
color2="#BBBBBB"
bgmerge_color="#EEEEEE"

ylabel_ins='Insertion time (min)'
ylabel_comp='Compaction time (min)'
ylabel_io='I/O time (min)'
ylabel_gb='Total data transferred (GB)'
ylabel_gb_diff='Data transferred (GB)'
ylabel_runs='Number of disk files'
ylabel_glatency='Get latency (ms)'
ylabel_gthrput='Get throughput (req/s)'

xlabel_time='Time (min)'
xlabel_datains_gb='Data inserted (GB)'
xlabel_datains_perc='Data inserted (%)'
xlabel_datains_perc_geo='Data inserted (%) - Geometric r=2'
xlabel_datains_perc_rng='Data inserted (%) - Rangemerge'
xlabel_pthrput='Put throughput (req/s)'
xlabel_gthrput='Get throughput (req/s)'
xlabel_gsize='Range get size (keys retrieved)'
xlabel_gthreads='Number of get threads'
xlabel_keysord='Percentage of keys ordered (%)'
xlabel_keyszipf='{/Symbol a} parameter of Zipf distribution'
xlabel_memsize='Memory size (MB)'
xlabel_blocksize='Time (min)'
xlabel_flushmem='Time (min)'
