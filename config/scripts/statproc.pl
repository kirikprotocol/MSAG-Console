#!/usr/bin/perl
use strict;
use Net::FTP;
use Time::Local;

my @findDirs=qw(. ./conf ../conf);

if($0=~/(.*)[\\\/][^\\\/]+/)
{
  push @findDirs,$1;
  push @findDirs,$1.'/conf';
  push @findDirs,$1.'../conf';
}

my $cfgFileName='statproc.conf';
my $doUpload=0;
my $date=time;

for my $arg(@ARGV)
{
  if($arg eq 'upload')
  {
    $doUpload=1;
    next;
  }
  if($arg=~/^cfg=(.*)$/)
  {
    $cfgFileName=$1;
    next;
  }
  if($arg=~/date=(\d\d)[.-](\d\d)(?:[.-](\d\d(?:\d\d)?))?/)
  {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $date=timelocal(0,5,0,$1,$2-1,$3?length($3)==4?$3-1900:$3+100:$year)+24*60*60;
    next;
  }
  if($arg eq '-h' || $arg eq '--help')
  {
    print "Usage:\n$0 [date={date}] [cfg={filename}] [upload]\n";
    exit;
  }
  die "Unknown option $arg";
}

my $cfgFile;

$cfgFile=openfile($cfgFileName,@findDirs);

my $cfg={
  maxSmsSec=>200,
  inputFile=>'logs/smscstats.log',
  outputFile=>'stat.txt',
  ftpHost=>'ftpserver',
  ftpUserPass=>'user:pass',
  ftpDir=>'/pub',
};

while(<$cfgFile>)
{
  chomp;
  next if /^(#|$)/;
  unless(/\s*(\w+)\s*=\s*(.*?)\s*$/)
  {
    die "Unrecognized config line:$_";
  }
  unless(exists($cfg->{$1}))
  {
    die "Unknown config option $1";
  }
#  print "Set $1=$2\n";
  $cfg->{$1}=$2;
}

open(my $inFile,$cfg->{inputFile}) || die "Failed to open input file '".$cfg->{inputFile}."':$!";

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($date);
$mon+=1;

my $cd=sprintf("%02d-%02d",$mday,$mon);

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($date-60*60);

$mon+=1;

my $pd=sprintf("%02d-%02d",$mday,$mon);

my $drx;
if($pd ne $cd)
{
  $drx="$pd|$cd";
}else
{
  $drx=$pd;
}

my %hst;

while(<$inFile>)
{
  if(/\w\s+($drx)\s+(\d\d):\d\d:\d\d,\d{3}\s+\d+\s+timestat:\s+maxTotalCounter=(\d+),\s+maxStatCounter=(\d+)/)
  {

    my $h=$2-1;
    if($cd ne $pd)
    {
      if($1 eq $cd && $2==0)
      {
        $h=23;
      }else
      {
        next if $1 eq $cd;
      }
    }
    next if($h==-1);
    my $mxtc=$3;
    my $mxsc=$4;
    $hst{$h}->{mxtc}=$mxtc if $hst{$h}->{mxtc}<$mxtc;
    $hst{$h}->{mxsc}=$mxsc if $hst{$h}->{mxsc}<$mxsc;
  }
}

my $outFileName=$cfg->{outputFile};

$outFileName=~s/\./-$pd./;

#print "outFN=$outFileName\n";

open(F,'>'.$outFileName) || die "Faield to open file '".$outFileName."':$!";

for my $h(0..23)
{
  next unless exists $hst{$h};
            #date           h   y   m   wd  wk  tot max usage%
  printf F "%02d.%02d.%02d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f%%\n",$mday,$mon,$year%100,$h,
    $year+1900,$mon,$wday,$yday/7,$hst{$h}->{mxsc},$cfg->{maxSmsSec},$hst{$h}->{mxsc}/$cfg->{maxSmsSec}*100.0;
}

close F;

if($doUpload)
{
  my $ftp=Net::FTP->new($cfg->{ftpHost},Debug=>0) or die "Cannot connect to ".$cfg->{ftpHost};
  $ftp->login(split(':',$cfg->{ftpUserPass})) or die "Login failed:".$ftp->message;
  $ftp->cwd($cfg->{ftpDir}) or die "Failed to cwd:".$ftp->message;
  $ftp->put($outFileName) or die "Failed to put file:".$ftp->message;
  $ftp->quit;
}

sub openfile{
  my ($fn,@dirs)=@_;
  for(@dirs)
  {
    if(-f $_.'/'.$fn  && open(my $f,$_.'/'.$fn))
    {
      return $f;
    }
  }
  die "Failed to open file '$fn' in dirs:".join(',',@dirs);
}
