
package    String::Between;
require    Exporter;

use strict;

our @ISA = qw(Exporter);
our @EXPORT = qw(between);
our $VERSION = 1.0;

sub between {
    my $where1 = index($_[0], $_[1]);
    my $where2 = index($_[0], $_[2], $where1+1);
    substr($_[0], $where1+1, $where2-$where1-1);
}
1
