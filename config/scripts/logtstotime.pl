#! /usr/bin/perl

my $in,$out;
if(@ARGV==2)
{
  open($in,$ARGV[0]) || die "$!";
  open($out,'>'.$ARGV[1]) || die "$!";
}else
{
  $in=*STDIN;
  $out=*STDOUT;
}
while(<$in>)
{
  s/\[(\d+)\s+(\d+\.\d+)\]/[$1 @{[''.localtime($2)]}]/;
  print $out $_;
}
