#!/bin/sh

filename=`expr //$1 : '.*/\(.*\)'`
fullpath=$1

if [ ! -f neu_fit.x ]; then
    cd neuz
    make
    cp neu_fit.x ..
    cd ..
fi

if [ ! -f 0.wts ]; then
    cp trainingData/0.wts .
fi

if [ ! -f run.param ]; then
    cp trainingData/run.param .
fi

if [ ! -f e4.x ]; then
    cd nne
    make
    cp e4.x ..
    cd ..
fi

if [ ! -f e4train.tbl ]; then
    cp trainingData/e4train.tbl .
fi

awk '{print 0.0,0.0,$7,$8,$9,$10,$11,0.0,0.0,0.0,0.0,0.0}' $fullpath > temp.nfit
sed 's/-9999/35/g' temp.nfit >temp2.nfit
mv temp2.nfit temp.nfit

./neu_fit.x 0.wts temp.nfit

mv bzphot.tbl $filename.zphot

./e4.x e4train.tbl $filename.zphot

#mv output.etbl $filename.etbl

pr -m -T -w 1000 $fullpath output.etbl | awk '{print $1,$5,$6,$4,$17,$18}' > output2.etbl

mv output2.etbl $filename.etbl

rm temp.nfit
rm output.etbl
rm $filename.zphot
