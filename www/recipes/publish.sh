dir=$1

if [[ $dir == "" ]]; then
    echo "usage: bash publish.sh dir"
    exit 45
fi

if [[ ! -e $dir ]]; then
    echo "making dir $dir"
    mkdir -p "$dir"
fi

echo Publishing to directory $dir

rsync -av \
    --include "*.py" \
    --include "*.htm" \
    --include "*.html" \
    --include "/styles" \
    --include "/styles/*" \
    --exclude ".*" \
    --exclude "*" \
    ./ "$dir/"

chmod 711 "$dir"
cd $dir
chmod go-rw cgi_* cp.py create_newdb.py recipe_list.py
