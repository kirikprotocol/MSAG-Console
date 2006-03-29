#!/bin/perl
use strict;
use File::stat;

if(@ARGV<3)
{
  print STDERR "Usage $0 indir inregexp outmask\n";
  exit;
}

my ($dir,$rx,$outmask)=@ARGV;

$dir.='/' unless $dir=~m!/$!;

my $foundSomething=0;
do{
$foundSomething=0;
opendir(D,$dir) || die "Failed to read dir $dir";
for my $file(readdir(D))
{
  next unless $file=~/$rx/;
  $foundSomething=1;
  my $st=stat($dir.$file);
  my $outfile=MakeFilename($outmask,$st->ctime);
  my $tempfile=$dir.$file.'tmp'.time;
  rename($dir.$file,$tempfile) || die "Failed to rename $file to $tempfile";
  my $cnt=1;
  while(-f $outfile)
  {
    $outfile=~s/(.*)\./$1\.$cnt\./ unless $outfile=~s/(.*)\.(\d+\.)/$1\.$cnt\./;
    $cnt++;
  }
  system("gzip -nc $tempfile >$outfile")==0 || die "gzip failed:$!";
  unlink($tempfile);
}
closedir(D);
}while($foundSomething);

sub MakeFilename
{
  my ($mask,$time)=@_;
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($time);
  $year+=1900;
  $mon++;
  my $h=
  {
    yyyy=>$year,
    yy=>$year,
    MM=>$mon,
    dd=>$mday,
    hh=>$hour,
    mm=>$min,
    ss=>$sec
  };
  $mask=~s/\%([a-zA-Z]{2,4})/sprintf('%02d',$h->{$1})/eg;
  return $mask;
}
