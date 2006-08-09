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

my @severityNames=qw(1 2 3 4 5);

my $cfgTimeStamp;
my $cfgFileName=$ARGV[0];
LoadConfig();


sub LoadConfig
{
  my @st=stat($cfgFileName);
  if(!@st)
  {
    die "Failed to stat '$cfgFileName': '$!'";
  }
  if($cfgTimeStamp==$st[9])
  {
    return;
  };
  print "Loading config:$cfgFileName\n";
  $cfgTimeStamp=$st[9];

  @loadAvgThresholds=();
  #$loadAvgActive=undef;
  %mntThresholds=();
  #%mntActive=();


  open(my $cfg,$cfgFileName) || die "Failed to open '$cfgFileName': '$!'";
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
    }elsif(/^([^,]+),([^,]+),(\d+),(\d+),(\d+),(\d+),(\d+)$/)
    {
      $mntThresholds{$1}={name=>$2,thr=>[$3,$4,$5,$6,$7]};
    }else
    {
      print STDERR "Unknown line in config: $_";
    }
  }
  close($cfg);
}

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
  LoadConfig();
}

print "Finished\n";

sub CheckLoadAverage
{
  my ($la)=(`uptime`=~/load average:\s+\d+\.\d+,\s+(\d+\.\d+)/);
  my @tm=localtime(time);
  my $hour=$tm[2];
  my $thr;
  do{
    $thr=$loadAvgThresholds[$hour];
    $hour--;
    $hour=23 if($hour<0);
  }while(!$thr);

  my $percent=int(100*$la/$thr->{hundredPercentValue});

  print "Load average:$la ($percent\%)\n";

  my $loadLevel;
  for(my $i=4;$i>=0;$i--)
  {
    if($percent>=$thr->{severityThresholds}->[$i])
    {
      $loadLevel=$i+1;
      last;
    }
  }
  if(!defined($loadLevel))
  {
    if($loadAvgActive)
    {
      SnmpTrap("Cleared OS AVG_LOAD Threshold crossed $percent\% (AlarmID=AVG_LOAD; severity=1)",1,'OS','AVG_LOAD');
      $loadAvgActive=undef;
    }
  }else
  {
    if($loadAvgActive ne $severityNames[$loadLevel])
    {
      $loadAvgActive=$severityNames[$loadLevel];
      SnmpTrap("Active OS AVG_LOAD Threshold crossed $percent\% (AlarmID=AVG_LOAD; severity=$loadAvgActive)",$loadAvgActive,'OS','AVG_LOAD');
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
    my $svrt=$mntThresholds{$mnt}->{thr};
    my $name=$mntThresholds{$mnt}->{name};

    my $fillLevel;
    for(my $i=4;$i>=0;$i--)
    {
      if($capacity>=$svrt->[$i])
      {
        $fillLevel=$i+1;
        last;
      }
    }
    if(!defined($fillLevel))
    {
      if($mntActive{$mnt})
      {
        SnmpTrap("Cleared OS HDD_$name Threshold crossed $capacity\% (AlarmID=HDD_$name; severity=1)",1,'OS',"HDD_$name");
        $mntActive{$mnt}=undef;
      }
    }else
    {
      if($mntActive{$mnt} ne $severityNames[$fillLevel])
      {
        $mntActive{$mnt}=$severityNames[$fillLevel];
        SnmpTrap("Active OS HDD_$name Threshold crossed $capacity\% (AlarmID=HDD_$name; severity=@{[$mntActive{$mnt}]})",$mntActive{$mnt},'OS',"HDD_$name");
      }
    }
  }
}

sub SnmpTrap
{
  my ($msg,$severity,$object,$alarmId)=@_;
  system(
"snmptrap -v 2c -c ussdc  traphost '' SIBINCO-SMSC-MIB::smscAlertFFMR ".
"SIBINCO-SMSC-MIB::alertSeverity i $severity ".
"SIBINCO-SMSC-MIB::alertMessage s '$msg' ".
"SIBINCO-SMSC-MIB::alertObjCategory s '$object' ".
"SIBINCO-SMSC-MIB::alertId s '$alarmId'"
);
}
