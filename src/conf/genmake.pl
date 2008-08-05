#! /usr/bin/perl
use strict;

open(my $mkf,'>makefile.inc') || die "Failed to open makefile.inc for writing:'$!'";


my $mods=readmodules('modules-list');

my $cmpclr;
my $libclr;
my $lnkclr;
my $clrend;

my $cmdprefix='@';

my $silent=$ENV{SILENT} eq 'YES';

if($ENV{VERBOSE} eq 'YES')
{
  $cmdprefix='';
}

if($ENV{USECOLORS} eq 'YES')
{
  $cmpclr='\033[01;32m';
  $libclr='\033[01;33m';
  $lnkclr='\033[01;31m';
  $clrend='\033[00m';
}

sub readmodules{
  my $fn=shift;
  open(my $f,$fn) || die "Failed to open $fn:'$!'";
  my @rv=grep!/^\s*(?:#|$)/,map{s/[\x0d\x0a]//g;/^(\w+)/;$1}<$f>;
  return \@rv;
}

my @preamble = (
    '# --- architecture specific variables',
#    'ifeq ($(filter-out linux Linux,$(shell uname)),)',
    'override ECHO := echo -e',
#    'else',
#    'override ECHO := echo',
#    'endif',
    '');
for my $l (@preamble) 
{
    print $mkf $l."\n";
}


print $mkf "build: ".join(' ',@$mods)."\n\n";
generate('',$mods);
print $mkf "clean: ".join(' ',map{"$_.rclean"}@$mods)."\n\n";
close($mkf);


sub generate{
  my $prefix=shift;
  my $ml=shift;
  for my $m(@$ml)
  {
    my $moddeps;
    next if $m=~/^\s*(?:$|#)/;
    my $dirname=$prefix.(length($prefix)?'/':'').$m;
    my $modlib='lib'.$dirname;
    $modlib=~s!/!-!g;
    my $moddir=$modlib;
    $modlib.='.a';
    
    
    my @files;
    my %binsrc;
    if(-f $dirname.'/binaries-list')
    {
      open(my $f,$dirname.'/binaries-list');
      while(<$f>)
      {
        s/[\x0d\x0a]//g;
        next if /^\s*(?:$|#)/;
        my $ln=$_;
        my ($binname,$srcname,$libs)=split(/\s+/,$ln,3);
        $binsrc{$srcname.'.cpp'}=1;
        if($binname=~/^-/)
        {
          next;
        }
        my $rawlibs=$libs;
        $libs=~s/(\$\((\w+)\))/if(exists($ENV{$2})){$ENV{$2}}else{$1}/ge;
        my $libdeps;
        for my $l(split(/\s+/,$libs))
        {
          if($l=~/^\@(.*)/)
          {
            $libdeps.=" \$(SMSC_BUILDDIR)/bin/$1";
          }

          next unless $l=~/^-l/;
          $l=~s/^-l//;
          my $lp=$l;
          $lp=~s!-!/!g;
          unless(-d $lp)
          {
            next;
          }
          if(-f "$lp/.ignorelib")
          {
            next;
          }
          $libdeps.=' $(SMSC_BUILDDIR)/lib/lib'.$l.'.a';
        }
        $rawlibs=~s/\@\S+//g;
        $moddeps.=' $(SMSC_BUILDDIR)/bin/'.$binname;
        my $ldflags=readstring($dirname.'/.ldflags');
        print $mkf '$(SMSC_BUILDDIR)/bin/'.$binname.': $(SMSC_BUILDDIR)/obj/'.$moddir.'/'.$srcname.'.o'.$libdeps."\n";
        print $mkf "\t\@mkdir -p `dirname \$@`\n";
        print $mkf "\t\@\$(ECHO) '${lnkclr}Linking \$\@${clrend}'\n" unless $silent;
        print $mkf "\t$cmdprefix\$(CXX) \$(CXXFLAGS) $ldflags -o \$@ \$< \$(LDFLAGS) $rawlibs\n\n";
        srcrule($dirname,$srcname.".cpp",\@files);
        push @files,'$(SMSC_BUILDDIR)/bin/'.$binname;
      }
    }
    
    opendir(my $dir,$dirname);
    my @dirlist=grep/^[^.].*\.c(?:pp)?$/,readdir($dir);
    @dirlist=grep{!exists($binsrc{$_})}@dirlist;
    closedir($dir);
    
    if(@dirlist)
    {
      my @objs=@dirlist;
      s/\.\w+$/.o/for@objs;
      $_='$(SMSC_BUILDDIR)/obj/'.$moddir.'/'.$_ for@objs;
      
      push @files,'$(SMSC_BUILDDIR)/lib/'.$modlib;
      
      print $mkf '$(SMSC_BUILDDIR)/lib/'.$modlib.':'.join(' ',@objs)."\n";
      print $mkf "\t\@\$(ECHO) '${libclr}Assembling $modlib${clrend}'\n" unless $silent;
      print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/lib/\n";
      print $mkf "\t\@rm -f \$(SMSC_BUILDDIR)/lib/$modlib\n";
      print $mkf "\t$cmdprefix\$(AR) -r \$(SMSC_BUILDDIR)/lib/$modlib ".join(' ',@objs)."\n\n";
      srcrule($dirname,$_,\@files)for@dirlist;
      $moddeps.=' $(SMSC_BUILDDIR)/lib/'.$modlib;
    }
    
    my $modname=$dirname;
    $modname=~s!/!.!g;
    print $mkf "$modname.clean:\n";
    print $mkf "\t\@\$(ECHO) '${lnkclr}Cleaning $modname${clrend}'\n";
    print $mkf "\t\@rm -f $_\n" for @files;
    print $mkf "\n";
    
    if(-f $dirname.'/version.inc')
    {
      print $mkf "$modname.newbuild:\n";
      print $mkf "\t\@perl conf/newbuild.pl $modname\n\n";
    }
    
    
    my $mods=[];
    if(-f $dirname.'/modules-list')
    {
      $mods=readmodules($dirname.'/modules-list');
      $moddeps.=" $modname.$_" for @$mods;
      generate($dirname,$mods);
    }
    
    $moddeps.=' '.readstring($dirname.'/.depends') if -f $dirname.'/.depends';
    
    print $mkf "$modname: $moddeps\n\n" if $moddeps;
    
    print $mkf "$modname.rclean: $modname.clean".(join('',map{" $modname.$_.rclean"}@$mods))."\n\n";
  }
}


sub srcrule{
  my ($dirname,$srcname,$files)=@_;
  my $basename=$srcname;
  $basename=~s/\.c(?:pp)?$//;
  my $objdir='lib'.$dirname;
  $objdir=~s!/!-!g;
  push @$files,"\$(SMSC_BUILDDIR)/obj/$objdir/$basename.o";
  push @$files,"\$(SMSC_BUILDDIR)/deps/$dirname/$basename.dep";
  print $mkf "\$(SMSC_BUILDDIR)/obj/$objdir/$basename.o: \$(SMSC_SRCDIR)/$dirname/$srcname\n";
  print $mkf "\t\@\$(ECHO) '${cmpclr}Compiling $dirname/$srcname${clrend}'\n" unless $silent;
  print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/obj/$objdir\n";
  my $cppflags=readstring($dirname.'/.cxxflags');
  my $cflags=readstring($dirname.'/.cflags');
  if($srcname=~/\.cpp/)
  {
    print $mkf "\t$cmdprefix\$(CXX) \$(CXXFLAGS) $cppflags \$(COMPFLAGS) -c -o \$\@ \$<\n";
    print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/deps/$dirname\n";
    print $mkf "\t\@".($cppflags?"CXXFLAGS=\"\$(CXXFLAGS) $cppflags\" ":'')."conf/mkdeps.sh $objdir $dirname/$basename .cpp\n\n";
    print $mkf "-include \$(SMSC_BUILDDIR)/deps/$dirname/$basename.dep\n\n";
    
  }else
  {
    print $mkf "\t$cmdprefix\$(CC) \$(CFLAGS) $cflags \$(C_COMPFLAGS) -c -o \$\@ \$<\n";
    print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/deps/$dirname\n";
    print $mkf "\t\@".($cflags?"CFLAGS=\"\$(CFLAGS) $cflags\" ":'')."conf/mkdeps.sh $objdir $dirname/$basename .c\n\n";
    print $mkf "-include \$(SMSC_BUILDDIR)/deps/$dirname/$basename.dep\n\n";
  }
}

sub readstring{
  my $fn=shift;
  return undef unless -f $fn;
  open(my $f,$fn);
  my $rv=<$f>;
  $rv=~s/[\x0d\x0a]//g;
  return $rv;
}
