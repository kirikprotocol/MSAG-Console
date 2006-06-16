#!/usr/bin/perl
use strict;
use File::Copy;
if(@ARGV!=2)
{
  print "Usage: dirsync dir1 dir2\n";
  exit;
}

my $dir1=$ARGV[0];
my $dir2=$ARGV[1];
my (@d1,@d2);
$dir1.='/' unless $dir1=~/\/$/;
$dir2.='/' unless $dir2=~/\/$/;
scandir($dir1,\@d1);
scandir($dir2,\@d2);


sync($dir1,\@d1,$dir2,\@d2);
sync($dir2,\@d2,$dir1,\@d1);

sub scandir
{
  my ($dir,$arr,$prefix)=@_;
  opendir(D,$dir) || die "Failed to open dir $dir for reading:$!";
  my @content=readdir(D);
  closedir(D);
  for my $entry(@content)
  {
    if(-f $dir.$entry)
    {
      push @$arr,$prefix.$entry;
    }elsif(-d $dir.$entry)
    {
      next if $entry eq '.' || $entry eq '..';
      scandir($dir.$entry.'/',$arr,$prefix.$entry.'/');
    }
  }
}

sub mycopy
{
  my ($src,$dst)=@_;
  print "Copy $src => $dst\n";
  copy($src,$dst);
  my @st=stat($src);
  utime($st[8],$st[9],$dst);
}

sub sync
{
  my ($base1,$files1,$base2,$files2)=@_;
  my (%h1,%h2);
  $h1{$_}=1 for @$files1;
  $h2{$_}=1 for @$files2;
  for my $file(@$files1)
  {
    unless(exists($h2{$file}))
    {
      mycopy($base1.$file,$base2.$file);
    }else
    {
      my $t1=(stat($base1.$file))[9];
      my $t2=(stat($base2.$file))[9];
      if($t1>$t2)
      {
        mycopy($base1.$file,$base2.$file);
      }elsif($t2>$t1)
      {
        mycopy($base2.$file,$base1.$file);
      }
    }
  }
}
