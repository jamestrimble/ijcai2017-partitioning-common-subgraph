echo "Number of instances where at least one of clique, CP-FC, McClique didn't time out"
cat ../experiments/gpgnode-results/mcsplain/runtimes.data | awk 'NR>1 && ($2<1000000 || $3<1000 || $8<1000000)' | wc -l
echo "Number of these where McClique wan't strictly the fastest"
cat ../experiments/gpgnode-results/mcsplain/runtimes.data | awk 'NR>1 && ($2<1000000 || $3<1000 || $8<1000000) && ($2<=$8 || $3*1000<=$8)' | wc -l
