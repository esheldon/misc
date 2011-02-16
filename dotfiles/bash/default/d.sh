test=$(echo $PATH | grep dmd2)
if [ "$test" == "" ]; then
    export PATH=~/local/dmd2/linux/bin:${PATH}
fi
