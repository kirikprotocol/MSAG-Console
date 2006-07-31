#!/usr/bin/perl
use strict;

if(@ARGV==0)
{
  die "Config file expected in command line";
}

my @loadAvgThresholds;
my $loadAvgActive;
my %mntThresholds;
my %mntActive;

my @severityNames=qw(NORMAL WARNING MINOR MAJOR CRITICAL);

open(my $cfg,$ARGV[0]) || die "Failed to open '$ARGV[0]': '$!'";
while(<$cfg>)
{
  s/[\x00-\x1f]//g;
  next if /^#/ || /^$/;
  if(/^(\d+),(\d+(?:\.\d+)?),(\d+),(\d+),(\d+),(\d+),(\d+)$/)
  {
    $loadAvgThresholds[$1]=
    {
      hundredPercentValue=>$2,
      severityThresholds=>[$3,$4,$5,$6,$7]
    };
  }elsif(/^([^,]+),(\d+),(\d+),(\d+),(\d+),(\d+)$/)
  {
    $mntThresholds{$1}=[$2,$3,$4,$5,$6];
  }else
  {
    print STDERR "Unknown line in config: $_";
  }
}
close($cfg);

unless(@loadAvgThresholds)
{
  die "Load average thresholds not specified";
}

my $running=1;

$SIG{TERM}=sub{$running=0;};
$SIG{INT}=sub{$running=0;};

while($running)
{
  CheckLoadAverage();
  CheckDiskFree();
  sleep(5);
}

print "Finished\n";

sub CheckLoadAverage
{
  my ($la)=(`uptime`=~/load average:\s+\d+\.\d+,\s+(\d+\.\d+)/);
  print "Load average:$la\n";
  my @tm=localtime(time);
  my $hour=$tm[2];
  my $thr;
  do{
    $thr=$loadAvgThresholds[$hour];
    $hour--;
    $hour=23 if($hour<0);
  }while(!$thr);

  my $percent=int(100*$la/$thr->{hundredPercentValue});

  my $loadLevel;
  for(my $i=4;$i>=0;$i--)
  {
    if($percent>=$thr->{severityThresholds}->[$i])
    {
      $loadLevel=$i;
      last;
    }
  }
  if(!defined($loadLevel))
  {
    if($loadAvgActive)
    {
      SnmpTrap("<CLEARED> <NORMAL> AVG_LOAD threshold crossed");
      $loadAvgActive=undef;
    }
  }else
  {
    if($loadAvgActive ne $severityNames[$loadLevel])
    {
      $loadAvgActive=$severityNames[$loadLevel];
      SnmpTrap("<ACTIVE> <$loadAvgActive> AVG_LOAD threshold crossed");
    }
  }
}

sub CheckDiskFree
{
  my @dfInfo=`df -k`;
  for(@dfInfo)
  {
    s/[\x0d\x0a]//g;
    my @line=split(/\s+/,$_);
    my $mnt=$line[5];
    next unless exists($mntThresholds{$mnt});
    my $capacity=int($line[4]);
    my $disk=$line[0];
    print "mnt=$mnt,cap=$capacity,dsk=$disk\n";
    my $svrt=$mntThresholds{$mnt};

    my $fillLevel;
    for(my $i=4;$i>=0;$i--)
    {
      if($capacity>=$svrt->[$i])
      {
        $fillLevel=$i;
        last;
      }
    }
    if(!defined($fillLevel))
    {
      if($mntActive{$mnt})
      {
        SnmpTrap("<CLEAR> <NORMAL> HDD threshold for $mnt crossed");
        $mntActive{$mnt}=undef;
      }
    }else
    {
      if($mntActive{$mnt} ne $severityNames[$fillLevel])
      {
        $mntActive{$mnt}=$severityNames[$fillLevel];
        SnmpTrap("<ACTIVE> <@{[$mntActive{$mnt}]}> HDD threshold for $mnt crossed");
      }
    }
  }
}

sub SnmpTrap
{
  print "SnmpTrap:",@_,"\n";
}
