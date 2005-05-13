#!/usr/bin/perl
use strict;
use IO::Socket::INET;
use IO::Select;

if(@ARGV!=2)
{
  print "Usage: exportstats.pl host:port user:password\n";
  exit(-1);
}

my $sck=IO::Socket::INET->new(PeerHost => $ARGV[0],
                              Proto    => 'tcp'
                              );
die "Connect failed:$!" unless $sck;
die "Username:password expected" unless $ARGV[1]=~/^(.+):(.+)$/;
getline($sck);
$sck->write($1."\n");
getline($sck);
$sck->write($2."\n");
getline($sck);
my @tm=localtime;
my $date=sprintf("%02d.%02d.%04d",$tm[3],$tm[4]+1,$tm[5]+1900);
$sck->write("export stats for $date\n");
getline($sck);
$sck->write("quit");
exit(0);

sub getline{
  my $l=readline($_[0]);
  die "$!" unless defined($l);
  die "Error response:$l" unless $l=~/^\+/;
  print "$l";
}
