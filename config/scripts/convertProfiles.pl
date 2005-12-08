use strict;

if(@ARGV!=2)
{
  print "Usage: $0 infile outfile\n";
  exit;
}

open(my $f,$ARGV[0]) or die "Failed to open @{[$ARGV[0]]}:$!";
my @hdr=split(/\t/,<$f>);
open(my $g,'>'.$ARGV[1]) or die "Failed to open @{[$ARGV[1]]}:$!";
my %unique;
while(<$f>)
{
  chomp;
  my @rec=split(/\t/);
  next unless @rec;
  my %rec;
  for(my $i=0;$i<@hdr;$i++)
  {
    $rec{$hdr[$i]}=$rec[$i];
  }
  next if $rec{TON_FULL}!=1 || $rec{NPI_FULL}!=1 || $rec{ADDR_FULL}=~/\?/;
  next unless $rec{ADDR_FULL}=~/^7/;
  next if $rec{TON_SHORT}!=$rec{TON_FULL} || $rec{NPI_SHORT}!=$rec{NPI_FULL};
  if(exists($unique{$rec{ADDR_FULL}}))
  {
    print "Duplicate:$rec{ADDR_FULL}:\n$_:\n$unique{$rec{ADDR_FULL}}\n";
    next;
  }else
  {
    $unique{$rec{ADDR_FULL}}=$_;
  }
  print $g qq[add profile .$rec{TON_FULL}.$rec{NPI_FULL}.$rec{ADDR_FULL} ].
           qq[report @{[$rec{TYP}&8?'full':$rec{TYP}&4?'final':'none']} ].
           qq[locale @{[$rec{LANG} eq 'rus'?'ru_ru':'en_en']} ].
           qq[encoding @{[$rec{TYP}&1?'ucs2':'default']} \n];
}
close($f);
close($g);