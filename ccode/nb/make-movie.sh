cd tests

#mencoder mf://*.png -mf w=800:h=600:fps=25:type=png -ovc copy -oac copy -o output.avi

#mencoder mf://*.png -mf w=800:h=600:fps=25:type=png -ovc lavc \
#        -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o output.avi

#mencoder "mf://*.png" -mf fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4 -of avi -o sortie.avi

#mencoder mf://*.png -mf fps=25:type=png -ovc raw -oac copy -o output.avi
#mencoder "mf://*.png" -o output.avi -fps 25 -ovc x264 -x264encopts subq=6:frameref=6:bitrate=75:me=umh:bframes=1:cabac:deblock -vf scale=320:200

#mencoder "mf://...*.png" -mf fps=30 -o ....mp4 -vf scale=1024:768,harddup -of lavf -lavfopts format=mp4 -ovc x264 -sws 9 -x264encopts nocabac:level_idc=30:bframes=0:bitrate=5000:threads=auto:turbo=1:global_header:threads=auto:subq=5:frameref=6:partitions=all:trellis=1:chroma_me:me=umh

ffmpeg -i test-nb-%06d.png nb-movie.mp4
#test-nb-002479.png
