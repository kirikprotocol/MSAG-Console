#! /usr/bin/perl
#
# "@(#)$Id$"
#
use strict;

open(my $mkf,'>makefile.inc') || die "Failed to open makefile.inc for writing:'$!'";


my $mods=readmodules('modules-list');

my $cmdprefix='@';
my $verboseflags='';

my $silent=$ENV{SILENT} eq 'YES';

if($ENV{VERBOSE} eq 'YES')
{
  $cmdprefix='';
#  $verboseflags='-v -v -v'
}

sub readmodules{
  my $fn=shift;
  open(my $f,$fn) || die "Failed to open $fn:'$!'";
  my @ml=grep!/^\s*(?:#|$)/,map{s/[\x0d\x0a]//g;/^([\w\?\!]+)/;$1}<$f>;
  my @rv;
  for(@ml)
  {
    if(/(\w+)\?(!)?(\w+)/)
    {
      if((exists($ENV{$3}) && $2 ne '!') || (!exists($ENV{$3}) && $2 eq '!' ))
      {
        push @rv,$1;
      }
    }else
    {
       push @rv,$_;
    }
  }
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
CXXCLR='.chr(27).'[01;36m
LIBCLR='.chr(27).'[01;34m
LNKCLR='.chr(27).'[01;35m
CLREND='.chr(27).'[00m
override CCXX:=conf/colorederr $(CXX)
override CCC:=conf/colorederr $(CC)
endif

';

print $mkf "build: ".join(' ',@$mods)."\n\n";
generate('',$mods);
print $mkf "clean: ".join(' ',map{"$_.rclean"}@$mods)."\n\n";
close($mkf);


sub generate{
  my $prefix=shift;
  my $ml=shift;
  # print STDERR "generate dir=$prefix\n";
  for my $m(@$ml)
  {
    # print STDERR "generate mod=$m\n";
    my $moddeps;
    next if $m=~/^\s*(?:$|#)/;
    $m=~s/^\?//;
    my $dirname=$prefix.(length($prefix)?'/':'').$m;
    my $modlib='lib'.$dirname;
    $modlib=~s!/!-!g;
    my $moddir=$modlib;
    $modlib.='.a';

    if(-f $dirname.'/.makefile')
    {
      print $mkf "-include $dirname/.makefile\n\n";
    }
    
    my @files;
    my %binsrc;
    if(-f $dirname.'/binaries-list')
    {
      open(my $f,$dirname.'/binaries-list');
      my $line = "";
      # switchMode:
      # 0 - not in switch;
      # 1 - in switch, value is not found yet;
      # 2 - in switch, under value found;
      # 3 - in switch, value found, but now we are not under it.
      my $switchMode;
      my $switchValue;
      my %localVars;
      while(<$f>)
      {
        s/[\x0d\x0a]//g;
        next if /^\s*$/;
        my $ln = $_;

        # print STDERR "ln=[$ln]\n";
        if($ln=~/^switch\s+(.*)$/)
        {
          $switchMode=1;
          $switchValue=$ENV{$1};
          # print STDERR "switch on $1, value=$switchValue\n";
          next;
        }
        if($ln=~/^end switch$/ || $ln=~/^endswitch$/ )
        {
          # print STDERR "end switch\n";
          $switchMode=0;
          next;
        }
        if($switchMode)
        {
          if ($ln=~/(.*?):(.*)/)
          {
            if($1 eq $switchValue)
            {
              $ln=$2;
              $switchMode=2;
              # print STDERR "switch matched: $1 -> ln=[$ln]\n";
            }elsif($1 eq 'default' && $switchMode==1)
            {
              $ln=$2;
              $switchMode=2;
              # print STDERR "switch matched default: -> ln=[$ln]\n";
            }else{
              # wrong switch
              if ($switchMode==2) {
                  $switchMode=3;
              }
              next;
            }
          }
          if ($switchMode!=2) {
              next;
          }
          # print STDERR "passed switch: ln=[$ln]\n";
        }

        if (length($line)>0 && substr($line,-1) eq '\\')
        {
            $line = substr($line,0,-1) . $ln;
        } else {
            $line = $ln;
        }
        if (length($line)>0 && substr($line,-1) eq '\\' )
        {
            next;
        }
        $line=~s/^\s+//;
        $line=~s/#.*$//;
        if ( $line eq '' )
        {
            next;
        }
        if($line=~/^(\w+)=(.*)$/)
        {
	  my $name=$1;
          my $value=$2;
          $value=~s/\%\((\w+)\)/if(exists($localVars{$1})){$localVars{$1};}else{print STDERR "Warning: undefined local variable $1\n";"";}/ge;
          $localVars{$name}=$value;
          next;
        }
        # print STDERR "line to parse: $line\n";
        my ($binname,$srcname,$libs) = split(/\s+/,$line,3);
        $line = "";
        $binsrc{$srcname.'.cpp'}=1;
        if($binname=~/^-/)
        {
          next;
        }
        $libs=~s/\%\((\w+)\)/if(exists($localVars{$1})){$localVars{$1};}else{print STDERR "Warning: undefined local variable $1\n";"";}/ge;
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
        $libdeps=' '.join(' ',reverse(split(/ /,$libdeps)));
        $rawlibs=~s/\@\S+//g;
        $moddeps.=' $(SMSC_BUILDDIR)/bin/'.$binname;
        my $ldflags=readstring($dirname.'/.ldflags');
        print $mkf '$(SMSC_BUILDDIR)/bin/'.$binname.': $(SMSC_BUILDDIR)/obj/'.$moddir.'/'.$srcname.'.o'.$libdeps." makefile.inc\n";
        print $mkf "\t\@mkdir -p `dirname \$@`\n";
        print $mkf "\t\@\$(ECHO) '\$(LNKCLR)Linking \$\@\$(CLREND)'\n" unless $silent;
        print $mkf "\t\$(PFX)\$(INSTRUMENTATION) \$(CXX) $verboseflags \$(CXXFLAGS) $ldflags -o \$@ \$< \$(LDFLAGS) $rawlibs\n\n";
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
      print $mkf "\t\$(PFX)\$(AR) -r \$(SMSC_BUILDDIR)/lib/$modlib ".join(' ',@objs)." 2\>/dev/null\n\n";
      srcrule($dirname,$_,\@files)for@dirlist;
      $moddeps.=' $(SMSC_BUILDDIR)/lib/'.$modlib;
    }
    
    my $modname=$dirname;
    $modname=~s!/!.!g;
    print $mkf "$modname.clean:\n";
    print $mkf "\t\@\$(ECHO) '\$(LNKCLR)Cleaning $modname\$(CLREND)'\n";
    print $mkf "\t\@rm -f $_\n" for @files;
    print $mkf "\n";
    
    if(-f $dirname.'/.version')
    {
      open(my $vf,$dirname.'/.version');
      my $vars={};
      while(<$vf>)
      {
        s/[\x0d\x0a]//g;
        s/^\s+//;
        s/\s+$//;
        next unless $_;
        next if /^#/;
        my ($n,$v)=split(/\s*=\s*/,$_);
        $vars->{$n}=$v;
      }
      for my $var(qw(PROD_PREFIX PROD_VER_FILE BUILDID_HEADER))
      {
        unless(exists($vars->{$var}))
        {
          die "Var $var not found in $dirname/.version\n";
        }
      }
      system('conf/gen_build_id.sh',$dirname,$vars->{PROD_PREFIX}, $vars->{PROD_VER_FILE},$vars->{BUILDID_HEADER});
      print $mkf "$modname.genbuildId:\n";
      print $mkf "\t".'@$(SMSC_SRCDIR)/conf/gen_build_id.sh '.
        join(' ',($dirname,$vars->{PROD_PREFIX},$vars->{PROD_VER_FILE},$vars->{BUILDID_HEADER}))." NO\n\n";
      print $mkf "$modname.check.buildId:\n";
      print $mkf "\t".'@$(SMSC_SRCDIR)/conf/gen_build_id.sh '.
        join(' ',($dirname,$vars->{PROD_PREFIX},$vars->{PROD_VER_FILE},$vars->{BUILDID_HEADER}))."\n\n";
      print $mkf '$(SMSC_SRCDIR)/'."$dirname/".$vars->{PROD_VER_FILE}.": $modname.check.buildId\n";
      print $mkf "$modname.clean: $modname.clean.buildId\n";
      print $mkf "$modname.clean.buildId:\n";
      print $mkf "\t".'@rm -f $(SMSC_BUILDDIR)/deps/'.$dirname.'/'.$vars->{BUILDID_HEADER}."\n\n";
    }
    
    my $mods=[];
    my $allmoddeps = $moddeps;
    if(-f $dirname.'/modules-list')
    {
      $mods=readmodules($dirname.'/modules-list');
      # print STDERR "modlist: [".join(',',@$mods)."]\n";
      for(@$mods)
      {
        unless(/^\?/)
        {
          $moddeps.=" $modname.$_";
        } else {
            s/^\?//;
            $allmoddeps.=" $modname.$_.all";
        }
      }
      generate($dirname,$mods);
    }
    
    $moddeps.=' '.readstring($dirname.'/.depends') if -f $dirname.'/.depends';
    $allmoddeps.=' '.readstring($dirname.'/.depends') if -f $dirname.'/.depends';
    
    if (-x $dirname.'/regtest.sh')
    {
        if ( $moddeps ) {
            print $mkf "$modname.regtest: $modname\n";
        } else {
            print $mkf "$modname.regtest:\n";
        }
        print $mkf "\t\@\$(ECHO) '\$(LNKCLR)Regression tests for $modname\$(CLREND)'\n";
        print $mkf "\t-mkdir -p \$(SMSC_BUILDDIR)/regtest/$modname\n";
        print $mkf "\tcp \$(SMSC_SRCDIR)/conf/regtest.logger.properties \$(SMSC_BUILDDIR)/regtest/$modname/logger.properties\n";
        print $mkf "\tcd \$(SMSC_BUILDDIR)/regtest/$modname ; LD_LIBRARY_PATH=\$\${LD_LIBRARY_PATH}:\$(SMSC_BUILDDIR)/bin \$(SMSC_SRCDIR)/$dirname/regtest.sh \$(SMSC_SRCDIR) \$(SMSC_BUILDDIR) $dirname $modname >regtest.output\n";
        print $mkf "\tdiff -u \$(SMSC_SRCDIR)/$dirname/regtest.output \$(SMSC_BUILDDIR)/regtest/$modname/regtest.output\n";
        print $mkf "\n";
    }

    print $mkf "$modname: $moddeps\n\n" if $moddeps;
    print $mkf "$modname.all: $allmoddeps\n\n" if $allmoddeps;
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
    print $mkf "\t\$(PFX)\$(CCXX) \$(COMPFLAGS) \$(CXXFLAGS) $cppflags -c -o \$\@ \$<\n";
    print $mkf "\t\@mkdir -p \$(SMSC_BUILDDIR)/deps/$dirname\n";
    print $mkf "\t\@".($cppflags?"CXXFLAGS=\"\$(CXXFLAGS) $cppflags\" ":'')."conf/mkdeps.sh $objdir $dirname/$basename .cpp\n\n";
    print $mkf "-include \$(SMSC_BUILDDIR)/deps/$dirname/$basename.dep\n\n";
    
  }else
  {
    print $mkf "\t\$(PFX)\$(CCC) \$(C_COMPFLAGS) \$(CFLAGS) $cflags -c -o \$\@ \$<\n";
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
