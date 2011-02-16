if [[ "${#PERL5LIB}" != 0 ]]; then
    export PERL5LIB=${HOME}/perllib:${HOME}/perllib/sdss_inventory:${HOME}/CPAN:${PERL5LIB}
else
    export PERL5LIB=${HOME}/perllib:${HOME}/perllib/sdss_inventory:${HOME}/CPAN
fi

