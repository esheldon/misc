#!/bin/sh

programdir=/Applications/ffmpegX/ffmpegX.app//Contents/Resources
indir=/Volumes/LaCie\ Disk/data/DVDRip/SOPRANOS3_D2\ Title\ 1/VIDEO_TS
outdir=$indir

fname=SOPRANOS3_D2\ Title\ 1.VOB

infile=$indir/$fname
logfile=$outdir/$fname.avi-0.log
outfile=$outdir/$fname.avi 

startdate=`date`
echo Encoding beginning $startdate

# these are same command except for -pass 1 and -pass 2
$programdir/ffmpeg -i $infile -y -threads 2 -map 0.0:0.0 -f avi -vcodec xvid -aspect 4:3 -b 1488 -s 720x384 -r ntsc-film -g 240 -me epzs -qmin 2 -qmax 9 -hq -acodec mp3 -ab 192 -ar 48000 -ac 2  -map 0.1:0.1 -benchmark -pass 1 -passlogfile $logfile $outfile

$programdir/ffmpeg -i $infile -y -threads 2 -map 0.0:0.0 -f avi -vcodec xvid -aspect 4:3 -b 1488 -s 720x384 -r ntsc-film -g 240 -me epzs -qmin 2 -qmax 9 -hq -acodec mp3 -ab 192 -ar 48000 -ac 2  -map 0.1:0.1 -benchmark -pass 2 -passlogfile $logfile $outfile

# remove the log file; note the -0.log is added automatically
rm $logfile

donedate=`date`
echo Encoding completed on $donedate

exit

# Used for XFiles 4x3
$programdir/ffmpeg -i $infile -y -threads 2 -map 0.0:0.0 -f avi -vcodec xvid -aspect 4:3 -b 1654 -s 640x480 -r ntsc-film -g 240 -me epzs -qmin 2 -qmax 15 -hq -acodec mp3 -ab 192 -ar 48000 -ac 2  -map 0.1:0.1 -benchmark -pass 1 -passlogfile $logfile $outfile

$programdir/ffmpeg -i $infile -y -threads 2 -map 0.0:0.0 -f avi -vcodec xvid -aspect 4:3 -b 1654 -s 640x480 -r ntsc-film -g 240 -me epzs -qmin 2 -qmax 15 -hq -acodec mp3 -ab 192 -ar 48000 -ac 2  -map 0.1:0.1 -benchmark -pass 2 -passlogfile $logfile $outfile

$rm $logfile
