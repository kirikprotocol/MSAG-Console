#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);
use Fcntl ':flock';

my $f;
open($f,'>>/global/data/conf/scripts/bill_lock') || die "Failed to open lock file:$!";
flock($f,LOCK_EX) || die "Lock failed:$!";


if(@ARGV<4)
{
  print STDERR "usage: bill2cdr indir outdir tmpdir arcdir [nobilldir]\n";
  exit;
}

my $indir=$ARGV[0];
my $outdir=$ARGV[1];
my $tmpdir=$ARGV[2];
my $arcdir=$ARGV[3];
my $nobilldir=$ARGV[4];
for($indir,$outdir,$tmpdir,$arcdir,$nobilldir)
{
  if($_ && ! -d $_)
  {
    die "error: $_ doesn't exists\n";
  }
}
for($indir,$outdir,$tmpdir,$arcdir,$nobilldir)
{
  next unless $_;
  $_.='/' unless $_=~m!/$!;
}
opendir(D,$indir) or die "failed to read $indir";
my @dir=readdir(D);
closedir(D);

my @monthes=qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);

for(@dir)
{
  next unless $_=~/\.csv$/;
  my $infile=$indir.$_;
  my $ofn=$_;
#  $ofn=~s/\.csv$/\.001/;
  $ofn=~s/_//;
  $ofn=~/(\d+)/;
  my $timestamp=$1;
  $timestamp=~/(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})/;
  my $t1=timegm($6,$5,$4,$3,$2-1,$1-1900);
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($t1);

  $year+=1900;
  my $curmon=$monthes[$mon];
  $mon++;
  $timestamp=sprintf("%04d%02d%02d%02d%02d%02d",$year,$mon,$mday,$hour,$min,$sec);

  my $tmpfile=$tmpdir.$ofn;
  my $outfile=$outdir.
#              sprintf("USSDC_%02d%s%d_%02d&%02d&%02d_",$mday,$curmon,$year,$hour,$min,$sec).
              "USSDC_".
              $ofn;
  print "$outfile\n";

  eval{
    if(-s $infile)
    {
      process($infile,$tmpfile);
    }else
    {
      unlink($infile);
      next;
    }
  };
  if($@)
  {
    print STDERR "Processing error on file $infile:'$@'\n";
    rename($infile,$infile.'.err');
  }

  if(-f $tmpfile)
  {
    if(!move($tmpfile,$outfile))
    {
      unlink $tmpfile;
      die "failed to move $tmpfile to $outfile";
    }
  }
  my $arcout=sprintf("%s/%04d-%s",$arcdir,$year,$curmon);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  $arcout.=sprintf("/%02d",$mday);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  $arcout.=sprintf("/%02d",$hour);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  move($infile,$arcout);
}


sub getFileName{
  my $path=shift;
  if($path=~m!.*/([^/]+)!)
  {
    return $1;
  }
  return $path;
}


sub process{
  my ($inf,$outf)=@_;
  print "Processing $inf\n";
  copy ($inf, $outf);
}
