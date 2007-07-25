use strict;
use DBI;
use IO::Socket::INET;

use constant SMSXHOST      =>'phoenix';
use constant SMSXADMINPORT =>'9090';
use constant DATABASEHOST  =>'dbhost';
use constant DATABASEUSER  =>'user';
use constant DATABASEPASS  =>'password';

use constant SELECTQUERY   =>'SELECT abonent,cnt FROM sponsored';
use constant UPDATEQUERY   =>'UPDATE sponsored SET cur_cnt=cnt, today_cnt=cnt';


my $dbh=DBI->connect("DBI:mysql:database=smsx;host=".DATABASEHOST.';',DATABASEUSER,DATABASEPASS,undef);

die "Connect failed" unless $dbh;

my $sth=$dbh->prepare(SELECTQUERY);

die "Prepare failed:".$dbh->errstr unless $sth;

my $rc=$sth->execute(@_);
die "Execute failed:$rc" if $rc<0;

my $req;
my $cnt=0;
my $total;
my $totalOk;

eval
{
for(1..$sth->rows)
{
  my $row=$sth->fetchrow_arrayref;
  unless($req)
  {
    $req='<?xml version="1.0" encoding="windows-1251"?>'.
         '<!DOCTYPE command SYSTEM "file:///command.dtd">'.
         '<command component="SMSC" method="set_sponsored_values" name="call" returnType="string" service="SMSC">'.
         '<param name="list" type="stringlist">';
  }else
  {
    $req.=',';
  }
  $req.=$row->[0].','.$row->[1];
  $cnt++;
  if($cnt>=1000)
  {
    $req.='</param></command>';
    $total+=$cnt;
    if(SendReq($req))
    {
      $totalOk+=$cnt;
    }
    $req='';
    $cnt=0;
  }
}
if($req)
{
  $total+=$cnt;
  if(SendReq($req))
  {
    $totalOk+=$cnt;
  }
}
$sth->finish;
printf "Sponsored updated $totalOk/$total\n";
};

$dbh->do(UPDATEQUERY);

$dbh->disconnect;

sub SendReq{
  my $sock = IO::Socket::INET->new(PeerAddr => SMSXHOST,
                              PeerPort => SMSXADMINPORT,
                              Proto    => 'tcp');
  my ($req)=@_;
  $req=pack('N',length($req)).$req;
  my $i=0;
  while($i<length($req))
  {
    my $block=length($req)-$i<1024?length($req)-$i:1024;
    my $rv=syswrite($sock,substr($req,$i,$block));
    unless(defined($rv))
    {
      die "Failed to write to socket:$!";
    }
    $i+=$rv;
  }
  my $respLen;
  sysread($sock,$respLen,4);
  $respLen=unpack('N',$respLen);
  my $resp;
  sysread($sock,$resp,$respLen);
  return $resp=~/status="Ok"/?1:0;
#  print "resp:$resp\n";
}
