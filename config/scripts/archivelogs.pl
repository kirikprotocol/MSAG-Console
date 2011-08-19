#!/bin/perl
use strict;
use File::stat;

if(@ARGV<3)
{
  print STDERR "Usage $0 indir inregexp outmask\n";
  print STDERR "outmask values (file creation time):\n";
  print STDERR "\%yyyy - year\n";
  print STDERR "\%yy - last 2 digits of year\n";
  print STDERR "\%MM - month\n";
  print STDERR "\%dd - day\n";
  print STDERR "\%hh - hour\n";
  print STDERR "\%mm - minute\n";
  print STDERR "\%ss - seconds\n";
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
  my $st=stat($dir.$file);
  next unless $st;
  $foundSomething=1;
  my $outfile=MakeFilename($outmask,$st->mtime);
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
