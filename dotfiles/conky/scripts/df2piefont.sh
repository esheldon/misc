disk=$1

# different on ubuntu? How?
awk_prog="{if (\$6 == \"${disk}\") print \$3,\$2}"
python ~/.dotfiles/conky/scripts/perc2piefont.py $(df -P "$disk" | awk "$awk_prog")
