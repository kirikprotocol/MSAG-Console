#!/usr/bin/perl
use strict;
use IO::Socket::INET;
use IO::Select;

$|=1;
close(STDERR);
open(STDERR,'>&',\*STDOUT);

if(@ARGV!=3)
{
  print "Usage: consoleCmds.pl host:port user:password filename\n";
  exit(-1);
}

my $f;
open($f,$ARGV[2]) || die "Failed to open file '".$ARGV[2]."':$!";

my $sck=IO::Socket::INET->new(PeerHost => $ARGV[0],
                              Proto    => 'tcp'
                              );
die "Connect failed:$!\n" unless $sck;
die "Username:password expected\n" unless $ARGV[1]=~/^(.+):(.+)$/;
print "========================\n".localtime()."\n";
putline($sck,"login: user='$1', pass='$2';",1);#user
getline($sck,60);
while(<$f>)
{
  s/[\r\n]//g;
  next unless length($_);
  if(/^#/)
  {
    print "$_\n";
    next;
  }

  putline($sck,$_);
  getline($sck,60);
}
putline($sck,"quit");
exit(0);

sub getline
{
  my ($s,$to)=@_;
  $to||=60;
  my $sel=IO::Select->new();
  $sel->add($s);
  die "Read timed out" unless $sel->can_read($to);
  my $l=readline($_[0]);
  die "Read failed:$!" unless defined($l);
  $l=~s/[\r\n]//g;
  die "Error response:$l\n" unless $l=~/^EXECUTED: ErrorCode=0[;,]/;
  print "<$l\n";
}

sub putline
{
  my ($s,$l,$hide)=@_;
  $s->write($l."\n") || die "Socket write failed:$!";
  if($hide)
  {
    $l=~s/pass='.*?'/pass='********'/;
  }
  print ">$l\n";
}
