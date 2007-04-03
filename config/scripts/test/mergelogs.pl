use strict;

if(@ARGV!=3)
{
  print "Usage: $0 cmdlog incomlog outlog\n";
  exit;
}

open(my $cmd,$ARGV[0]) || die "Failed to open ".$ARGV[0].":$!\n";
open(my $inc,$ARGV[1]) || die "Failed to open ".$ARGV[1].":$!\n";
open(my $out,'>'.$ARGV[2]) || die "Failed to open ".$ARGV[2].":$!\n";

my $tsrx=qr(\d{2}\.\d{2}\.\d{4}\s+\d{2}:\d{2}:\d{2}.\d{3});

my $a0;
print $out $_ for sort{$a=~$tsrx;$a0=$&;$b=~$tsrx;$a0 cmp $&;}grep(/$tsrx/,(<$cmd>,<$inc>));
