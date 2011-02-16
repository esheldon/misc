sub between {
    my $where1 = index($_[0], $_[1]);
    my $where2 = index($_[0], $_[2], $where1+1);
    substr($_[0], $where1+1, $where2-$where1-1);
}
1
