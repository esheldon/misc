if [[ -e ~/.gem/ruby/1.8/bin ]]; then
    prepend_path PATH ~/.gem/ruby/1.8/bin
fi
/var/lib/gems/1.8/bin
if [[ -e  /var/lib/gems/1.8/bin ]]; then
    prepend_path PATH /var/lib/gems/1.8/bin
fi

export PATH
