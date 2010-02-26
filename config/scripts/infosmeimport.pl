#!/usr/bin/perl
use strict;
use IO::Socket::INET;
use IO::Select;

if(@ARGV!=3)
{
  print "Usage: infosmeimport.pl host:port user:password inputdir\n";
  exit(-1);
}

my $sck=IO::Socket::INET->new(PeerHost => $ARGV[0],
                              Proto    => 'tcp'
                              );
die "Connect failed:$!" unless $sck;
die "Username:password expected" unless $ARGV[1]=~/^(.+):(.+)$/;
getline($sck,60);
$sck->write($1."\n");
getline($sck,60);
$sck->write($2."\n");
getline($sck,60);

my $dir=$ARGV[2];
$dir.='/' unless $dir=~m!/$!;
opendir(D,$dir) || die "Failed to open dir $dir for reading:$!";
for my $file(readdir(D))
{
  next unless $file=~/.csv$/i;
  $sck->write("infosme import task $dir$file\n");
  getline($sck,10*60);
}
closedir(D);

$sck->write("quit");
exit(0);

sub getline{
  my ($s,$to)=@_;
  $to||=60;
  my $sel=IO::Select->new();
  $sel->add($s);
  die "Read timed out" unless $sel->can_read($to);
  my $l=readline($_[0]);
  die "$!" unless defined($l);
  die "Error response:$l" unless $l=~/^\+/;
  print "$l";
}
