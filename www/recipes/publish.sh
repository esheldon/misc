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
    --include "/styles" \
    --include "/styles/*" \
    --exclude ".*" \
    --exclude "*" \
    ./ "$dir/"

chmod 711 "$dir"
