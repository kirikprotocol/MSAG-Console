#! /usr/bin/perl

use strict;

my $dir=$ARGV[0];

die "Dir `$dir' not found" unless -d $dir;
exit unless -f "$dir/binaries-list";

open(IN,"$dir/binaries-list") or die "Failed to open $dir/binaries-list";

my $srcdir=$ENV{SMSC_SRCDIR};
my $builddir=$ENV{SMSC_BUILDDIR};

die "SMSC_SRCDIR not defined or dir doesn't exist!" unless -d $srcdir;
die "SMSC_BUILDDIR not defined" unless $builddir;

my $pdir=substr($dir,length($srcdir)+1);
my $lib=$pdir;
$lib=~s#/#-#g;

open(OUT,">>$builddir/deps/$pdir/.binaries-rules")
 or die "Failed to open $builddir/deps/$pdir/.binaries-rules";

my %ldeps;

while(<IN>)
{
  s/[\x0d\x0a]//gsm;
  next if /^#/;
  next if /^\s*$/;
  my @f=split(/\s+/);
  my $bin=shift @f;
  my $src=shift @f;
  my $libdeps='';
  for(@f)
  {
    my $l=$_;
    $l=~s/^-l//;
    my $lpath=$l;
    $lpath=~s!-!/!g;
#    print "lpath:$lpath\n";
    next unless -d "$srcdir/$lpath";
    #next if $lib eq $l;
    $libdeps.=" $builddir/lib/lib$l.a";
  }
  print OUT "$builddir/bin/$bin : $builddir/obj/lib$lib/$src.o$libdeps\n";
  print OUT "\tmkdir -p `dirname \$@`\n";
  print OUT "\techo linking \$@\n";
  print OUT "\t\$(CXX) \$(CXXFLAGS) -o \$@ \$< \$(LDFLAGS) @f\n\n";
  for my $l(@f)
  {
    $l=~s/^-l//;
    my $lpath=$l;
    $lpath=~s!-!/!g;
#    print "lpath:$lpath\n";
    next unless -d "$srcdir/$lpath";
    next if $lib eq $l;
    next if $ldeps{"$builddir/lib/lib$l.a"};
    $ldeps{"$builddir/lib/lib$l.a"}=1;
    print OUT "$builddir/lib/lib$l.a : __emptylib__\n";#.calclibdeps($l,"$srcdir/$lpath")."\n";
    print OUT "\t\@cd $srcdir/$lpath;make \$@\n\n";
  }
}
close OUT;
close IN;

sub calclibdeps{
  my ($l,$lib)=@_;
  opendir(D,$lib);
  my @s;
  for(readdir(D))
  {
    next if $_ eq '.' || $_ eq '..';
    next unless /\.cp?p?$/;
    push @s,$_;
  }
  open(F,$lib.'/binaries-list');
  while(<F>)
  {
    s/[\x0d\x0a]//gsm;
    next if /^#/;
    next unless $_=~/\S/;
    my($bin,$file)=split(" ");
    @s=grep !/$file\.cp?p?$/,@s;
  }
  #s/\.cp?p?$/.o/ for @s;
  for(@s)
  {
    m!^(.*)\.cp?p?!;
    $_="$builddir/obj/lib$l/$1.o";
  }
  return join(' ',@s);
}
