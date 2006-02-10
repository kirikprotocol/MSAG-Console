rm ./gt.log
./scag/test3 -a sunfire 29992
grep <gt.log >testif.txt "FROMRULE"
grep <gt.log >errorlist.txt "rror" 