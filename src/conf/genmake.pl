#! /usr/bin/perl
use strict;

open(my $mkf,'>makefile.inc') || die "Failed to open makefile.inc for writing:'$!'";


my $mods=readmodules('modules-list');

my $cmdprefix='@';

my $silent=$ENV{SILENT} eq 'YES';

if($ENV{VERBOSE} eq 'YES')
{
  $cmdprefix='';
}

sub readmodules{
  my $fn=shift;
  open(my $f,$fn) || die "Failed to open $fn:'$!'";
  my @rv=grep!/^\s*(?:#|$)/,map{s/[\x0d\x0a]//g;/^(\w+)/;$1}<$f>;
  return \@rv;
}

print $mkf
    '# --- architecture specific variables
    ifeq ($(filter-out linux Linux,$(shell uname)),)
    override ECHO := echo -e
    else
    override ECHO := echo
    endif
    ';
    
print $mkf
'
PFX=@

ifeq ($(VERBOSE),YES)
override PFX=
endif

CCXX:=$(CXX)
CCC:=$(CC)

ifeq ($(USECOLORS),YES)
CXXCLR=\033[01;36m
LIBCLR=\033[01;34m
LNKCLR=\033[01;35m
CLREND=\033[00m
override CCXX:=conf/colorederr $(CXX)
override CCC:=conf/clorederr $(CC)
endif

';

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
      my $line = "";
      while(<$f>)
      {
        s/[\x0d\x0a]//g;
        next if /^\s*(?:$|#)/;
        my $ln = $_;
        if ( '\\' eq substr($ln,-1) ) {
            $line .= substr($ln,0,length($ln)-1);
            next;
        }
        my ($binname,$srcname,$libs) = split(/\s+/,$line.$ln,3);
        $line = "";
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
        print $mkf "\t\@\$(ECHO) '\$(LNKCLR)Linking \$\@\$(CLREND)'\n" unless $silent;
        print $mkf "\t\$(PFX)\$(CXX) \$(CXXFLAGS) $ldflags -o \$@ \$< \$(LDFLAGS) $rawlibs\n\n";
        srcrule($dirname,$srcname.".cpp",\@files);
        push @files,'$(SMSC_BUILDDIR)/bin/'.$binname;
      } # while
      die "Abnormal end-of-file $dirname/binaries-list\nline=<$line>\n"
          if ( length($line) != 0 );
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
      print $mkf "\t\@\$(ECHO) '\$(LIBCLR)Assembling $modlib\$(CLREND)'\n" unless $silent;
      print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/lib/\n";
      print $mkf "\t\@rm -f \$(SMSC_BUILDDIR)/lib/$modlib\n";
      print $mkf "\t\$(PFX)\$(AR) -r \$(SMSC_BUILDDIR)/lib/$modlib ".join(' ',@objs)." 2>/dev/null\n\n";
      srcrule($dirname,$_,\@files)for@dirlist;
      $moddeps.=' $(SMSC_BUILDDIR)/lib/'.$modlib;
    }
    
    my $modname=$dirname;
    $modname=~s!/!.!g;
    print $mkf "$modname.clean:\n";
    print $mkf "\t\@\$(ECHO) '\$(LNKCLR)Cleaning $modname\$(CLREND)'\n";
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
  print $mkf "\t\@\$(ECHO) '\$(CXXCLR)Compiling $dirname/$srcname\$(CLREND)'\n" unless $silent;
  print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/obj/$objdir\n";
  my $cppflags=readstring($dirname.'/.cxxflags');
  my $cflags=readstring($dirname.'/.cflags');
  if($srcname=~/\.cpp/)
  {
    print $mkf "\t\$(PFX)\$(CCXX) \$(CXXFLAGS) $cppflags \$(COMPFLAGS) -c -o \$\@ \$<\n";
    print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/deps/$dirname\n";
    print $mkf "\t\@".($cppflags?"CXXFLAGS=\"\$(CXXFLAGS) $cppflags\" ":'')."conf/mkdeps.sh $objdir $dirname/$basename .cpp\n\n";
    print $mkf "-include \$(SMSC_BUILDDIR)/deps/$dirname/$basename.dep\n\n";
    
  }else
  {
    print $mkf "\t\$(PFX)\$(CCC) \$(CFLAGS) $cflags \$(C_COMPFLAGS) -c -o \$\@ \$<\n";
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
