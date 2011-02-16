if [ $# -gt 0 ]; then
	stats=$(free | awk '/Swap/ {print $3,$2}')
else
	stats=$(free | awk '/Mem/ {print $3-$6-$7, $2}')
fi

python ~/.dotfiles/conky/scripts/perc2piefont.py $stats
