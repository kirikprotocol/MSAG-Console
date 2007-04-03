use strict;
use Time::Local;

use constant MAXRESPDELAY=>1.0;
use constant MAXANSWERDELAY=>5.0;


if(@ARGV!=2)
{
  print "Usage: $0 log report\n";
  exit;
}

open(my $log,$ARGV[0]) || die "Failed to open ".$ARGV[0].":$!\n";
open(my $out,'>'.$ARGV[1]) || die "Failed to open ".$ARGV[1].":$!\n";

my $tsrx=qr((\d{2})\.(\d{2})\.(\d{4})\s+(\d{2}):(\d{2}):(\d{2}).(\d{3}));

my %vcs;

my %reg;

my $respsumm=0;
my $respmin=1e10;
my $respmax=0;
my $ansmin=1e10;
my $ansmax=0;
my $anssumm=0;
my $count=0;
my $respovercnt=0;
my $ansovercnt=0;

my ($start,$end);
my $sendCnt=0;

while(<$log>)
{
  if(/$tsrx\s+\{([^}]+)\}:PDU sent. Seq=(\d+)/)
  {
    my $ts=mktimestamp($3,$2,$1,$4,$5,$6,$7);
    $start=$ts unless defined($start);
    $end=$ts;
    $sendCnt++;
    my $abn=$8;
    my $seq=$9;
    $vcs{$abn}={} unless exists $vcs{$abn};
    if(exists($vcs{$abn}->{sent}))
    {
      #wtf?
    }
    $vcs{$abn}->{sent}=$ts;
    if(exists $reg{$seq})
    {
      if($reg{$seq}->{st}==0)
      {
        $vcs{$abn}->{resp}=$reg{$seq}->{ts};
      }else
      {
        delete $vcs{$abn};
        delete $reg{$seq};
        next;
      }
      delete $reg{$seq};
    }else
    {
      $reg{$seq}={abn=>$abn};
    }
    Check($vcs{$abn});
    next;
  }
  if(/$tsrx:Received async submit sm resp:status=(\d+), seq=(\d+), msgId=/)
  {
    my $ts=mktimestamp($3,$2,$1,$4,$5,$6,$7);
    my $st=$8;
    my $seq=$9;
    $reg{$seq}={} unless exists $reg{$seq};
    my $abn;
    if(exists($reg{$seq}->{abn}))
    {
      if($st!=0)
      {
        delete $vcs{$reg{$seq}->{abn}};
        delete $reg{$seq};
        next;
      }else
      {
        $abn=$reg{$seq}->{abn};
        $vcs{$abn}={} unless exists $vcs{$abn};
        $vcs{$abn}->{resp}=$ts;
        delete $reg{$seq};
      }
    }else
    {
      $reg{$seq}->{resp}={st=>$st,ts=>$ts};
    }
    Check($vcs{$abn}) if $abn;
    next;
  }
  if(/VCInfo\((?:dataSm|deliver)\): \[\d+\]$tsrx \{([^}]+)\}/)
  {
      #VCInfo(deliver): [0001]13.02.2007 14:29:20.682 {+79130000001}
    my $ts=mktimestamp($3,$2,$1,$4,$5,$6,$7);
    my $abn=$8;
    $vcs{$abn}->{answer}=$ts;
    Check($vcs{$abn});
  }
}

printf $out "Resp limit exceeded: %.02f%%\n",100.0*$respovercnt/$count;
printf $out "Answer limit exceeded: %.02f%%\n",100.0*$ansovercnt/$count;
printf $out "Resp min time:%d ms\n",int($respmin);
printf $out "Resp avg time:%d ms\n",int($respsumm/$count);
printf $out "Resp max time:%d ms\n",int($respmax);
printf $out "Answer min time:%d ms\n",int($ansmin);
printf $out "Answer avg time:%d ms\n",int($anssumm/$count);
printf $out "Answer max time:%d ms\n",int($ansmax);
printf $out "\nAverage sending speed:%.02f/sec\n",$sendCnt/(($end-$start)/1000);

sub Check{
  my $item=shift;
  if(exists($item->{sent}) && exists($item->{resp}) && exists($item->{answer}))
  {
    my $respdelay=$item->{resp}-$item->{sent};
    if($respdelay>MAXRESPDELAY*1000)
    {
      $respovercnt++;
    }
    my $ansdelay=$item->{answer}-$item->{sent};
    if($item->{answer}-$item->{sent}>MAXANSWERDELAY*5000)
    {
      $ansovercnt++;
    }
    $respmin=$respdelay if $respdelay<$respmin;
    $respmax=$respdelay if $respdelay>$respmax;
    $ansmin=$ansdelay if $ansdelay<$ansmin;
    $ansmax=$ansdelay if $ansdelay>$ansmax;
    $respsumm+=$respdelay;
    $anssumm+=$ansdelay;
    $count++;
    delete $item->{sent};
    delete $item->{resp};
    delete $item->{answer};
  }
}

sub mktimestamp
{
  my ($year,$mon,$day,$hour,$min,$sec,$msec)=@_;
  return timelocal($sec,$min,$hour,$day,$mon-1,$year-1900)*1000.0+$msec;
}