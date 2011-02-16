#!/bin/sh

#echo Clearing messages
rm ~/Mail/funandgames/*

#echo copying funandgames folder
cp ~/mail/funandgames /tmp/funandgames

inc -silent +funandgames -file /tmp/funandgames
rm ~/Mail/funandgames/1

rm /tmp/funandgames

# this is to create it the first time
#mhonarc -treverse -reverse -quiet -title "fun.and.games archive" -outdir ~/WWW/funandgames ~/Mail/funandgames

# this is to add
mhonarc -quiet -treverse -reverse -add Mail/funandgames/ -outdir ~/WWW/funandgames
