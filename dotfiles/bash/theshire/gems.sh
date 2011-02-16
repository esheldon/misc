test=$(echo $PATH | grep gem)
if [ "$test" == "" ]; then
    export PATH=~/.gem/ruby/1.8/bin:${PATH}
fi
