#!/usr/bin/perl
use strict;
use File::Copy;

if(@ARGV!=3)
{
  die "Config file expected in command line";
}

my @loadAvgThresholds;
my $loadAvgActive;
my %mntThresholds;
my %mntActive;

my $hostname=`hostname`;

my @severityNames=qw(1 2 3 4 5);

my $cfgTimeStamp;
my $cfgFileName=$ARGV[0];
my $csvFileDir=$ARGV[1];
$csvFileDir.='/' unless $csvFileDir=~m!/$!;
my $csvRolInterval=$ARGV[2];

my $csvFile;
my $csvFileName;
my $lastCsvRol=time;

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
  Log("Loading config:$cfgFileName");
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
    if(/^(\d+),(\d+(?:\.\d+)?),(\d+),(\d+),(\d+),(\d+)$/)
    {
      $loadAvgThresholds[$1]=
      {
        hundredPercentValue=>$2,
        severityThresholds=>[$3,$4,$5,$6]
      };
    }elsif(/^([^,]+),([^,]+),(\d+),(\d+),(\d+),(\d+)$/)
    {
      $mntThresholds{$1}={name=>$2,thr=>[$3,$4,$5,$6]};
    }else
    {
      print STDERR "Unknown line in config: $_\n";
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
  RollCsv();
}

Log("Finished");

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

  #print "Load average:$la ($percent\%)\n";

  my $loadLevel;
  for(my $i=3;$i>=0;$i--)
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
      SnmpTrap("CLEARED OS AVG_LOAD Threshold crossed $percent\% at $hostname (AlarmID=AVG_LOAD; severity=1)",1,'OS','AVG_LOAD');
      $loadAvgActive=undef;
    }
  }else
  {
    if($loadAvgActive ne $severityNames[$loadLevel])
    {
      $loadAvgActive=$severityNames[$loadLevel];
      SnmpTrap("ACTIVE OS AVG_LOAD Threshold crossed $percent\% at $hostname (AlarmID=AVG_LOAD; severity=$loadAvgActive)",$loadAvgActive,'OS','AVG_LOAD');
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
    #print "mnt=$mnt,cap=$capacity,dsk=$disk\n";
    my $svrt=$mntThresholds{$mnt}->{thr};
    my $name=$mntThresholds{$mnt}->{name};

    my $fillLevel;
    for(my $i=3;$i>=0;$i--)
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
        SnmpTrap("CLEARED OS HDD_$name Threshold crossed $capacity\% at $hostname (AlarmID=HDD_$name; severity=1)",1,'OS',"HDD_$name");
        $mntActive{$mnt}=undef;
      }
    }else
    {
      if($mntActive{$mnt} ne $severityNames[$fillLevel])
      {
        $mntActive{$mnt}=$severityNames[$fillLevel];
        SnmpTrap("ACTIVE OS HDD_$name Threshold crossed $capacity\% at $hostname (AlarmID=HDD_$name; severity=@{[$mntActive{$mnt}]})",$mntActive{$mnt},'OS',"HDD_$name");
      }
    }
  }
}

sub Log{
  my $str=shift;
  my @tm=localtime(time);
  printf("%02d-%02d %02d:%02d:%02d %s\n",$tm[4]+1,$tm[3],$tm[2],$tm[1],$tm[0],$str);
}

sub CsvLog{
  my ($msg,$severity,$object,$alarmId)=@_;
  my @tm=gmtime(time);
  unless(defined($csvFile))
  {
    $csvFileName=$csvFileDir.sprintf("%04d%02d%02d_%02d%02d%02d.csv",$tm[5]+1900,$tm[4]+1,$tm[3],$tm[2],$tm[1],$tm[0]);
    open($csvFile,'>'.$csvFileName) || die "Failed to open $csvFileName:$!";
    print $csvFile "SUBMIT_TIME,ALARM_ID,ALARMCATEGORY,SEVERITY,TEXT\n";
  }
  my $ts=sprintf("%02d.%02d.%04d %02d:%02d:%02d",$tm[3],$tm[4]+1,$tm[5]+1900,$tm[2],$tm[1],$tm[0]);
  print $csvFile qq!$ts,"$alarmId","$object",$severity,"$msg"\n!;
}

sub RollCsv{
  my $now=time;
  return unless $now-$lastCsvRol>$csvRolInterval;
  if(defined($csvFile))
  {
    close($csvFile);
    if(exists($ENV{HS_MIRROR_PATH}))
    {
      my $dstFn=$ENV{HS_MIRROR_PATH};
      $dstFn=$1 if $dstFn=~m!(.*)/$!;
      $dstFn.=$csvFileName;
      copy($csvFileName,$dstFn);
    }
  }
  $csvFile=undef;
  $lastCsvRol=$now;
}

sub SnmpTrap
{
  my ($msg,$severity,$object,$alarmId)=@_;
  CsvLog(@_);
  my $cmd="snmptrap -v 2c -c ussdc  traphost '' SIBINCO-SMSC-MIB::smscAlertFFMR ".
"SIBINCO-SMSC-MIB::alertSeverity i $severity ".
"SIBINCO-SMSC-MIB::alertMessage s '$msg' ".
"SIBINCO-SMSC-MIB::alertObjCategory s '$object' ".
"SIBINCO-SMSC-MIB::alertId s '$alarmId'";
  Log("exec:$cmd");
  system($cmd);
}
