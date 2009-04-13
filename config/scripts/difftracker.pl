#!/usr/bin/perl

use strict;
use File::Find;
use File::Copy;
use File::Basename;


use constant SRCDIR=>'/home/skv/temp/difftracker/src/';
use constant TMPDIR=>'/home/skv/temp/difftracker/tmp/';
use constant DSTDIR=>'/home/skv/temp/difftracker/dst/';
use constant TARPFX=>'changes';

my @changedFiles;

sub CompareSub{
  return unless -f $_;
  my $src=$_;
  my $tmp=$src;
  my $srcdir=SRCDIR;
  my $tmpdir=TMPDIR;
  $tmp=~s/$srcdir/$tmpdir/;
  my $code=system("diff $src $tmp >>/dev/null 2>>/dev/null");
  if($code!=0)
  {
    push @changedFiles,$src;
    my $dstdir=dirname $tmp;
    unless(-d $dstdir)
    {
      mkdir $dstdir;
    }
    copy($src,$tmp);
  }
}

find({wanted=>\&CompareSub,follow=>1,no_chdir=>1},SRCDIR);

if(@changedFiles)
{
  my $fn=DSTDIR.TARPFX;
  my @lt=localtime;
  $fn.=sprintf('%02d%02d%02d-%02d%02d%02d.tar',$lt[5]%100,$lt[4]+1,$lt[3],$lt[2],$lt[1],$lt[0]);
  my $srcdir=SRCDIR;
  for(@changedFiles)
  {
    s/$srcdir//;
  }
  chdir($srcdir);
  system("tar -cf $fn ".join(' ',@changedFiles));
}
