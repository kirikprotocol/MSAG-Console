#!/usr/bin/perl
use strict;
use Net::FTP;

use constant HOST=>'phoenix';
use constant USER=>'starkom';
use constant PASSWD=>'starkom';
use constant FTPDIR=>'ftptest';
use constant DSTDIR=>'.';

my $ftp=Net::FTP->new(HOST,Debug=>0) or die "Cannot connect to ".HOST;
$ftp->login(USER,PASSWD) or die "Login failed:".$ftp->message;
$ftp->cwd(FTPDIR) or die "Failed to cwd:".$ftp->message;
my @l=$ftp->ls();
chdir(DSTDIR);
for(@l)
{
	next unless /.csv$/;
	print "get $_\n";
	$ftp->get($_,$_.'.tmp') || die "Failed to get $_:".$ftp->message;
	print "delete $_\n";
	$ftp->delete($_) || die "Failed to rm $_:".$ftp->message;
        rename($_.'.tmp',$_);
}
$ftp->quit;
