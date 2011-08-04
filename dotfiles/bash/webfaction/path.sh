# don't want to re-read this for screen
case $TERM in
	screen*) return ;;
	*) ;;
esac

append_path PATH ~/python/bin
append_path PATH ~/shell_scripts
append_path PATH ~/shell_scripts/sdss
append_path PATH ~/perllib
append_path PATH ~/perllib/sdss_inventory

append_path PYTHONPATH ~/python

export PATH

