use strict;
use Cwd;
my $mod=$ARGV[0];
my $srcdir=$ENV{SMSC_SRCDIR};
if(!$srcdir || !-d $srcdir)
{
  print STDERR "SMSC_SRCDIR MUST BE DEFINED\n";
  exit(1);
}
unless(-d '../'.$mod)
{
  print STDERR "MODULE NAME DOESN'T MATCH DIRECTORY NAME!\n".cwd."\n";
  exit(1);
}

my $verfile;

if($mod eq 'webapp')
{
  $verfile='smsc/WEB-INF/src/ru/novosoft/smsc/util/Version.java';
}else
{
  $verfile='version.inc';
}

unless(-f $verfile)
{
  print STDERR "NO $verfile FOUND IN SELECTED MODULE!\n";
  exit(1);
}
open(F,$verfile) || die "Failed to open version.inc for reading";
my @l=<F>;
close F;
my $build;
if($mod eq 'webapp')
{
  for(@l)
  {
    if(/\s*public\s+static\s+final\s+int\s+VER_BUILD\s*=\s*(\d+);/)
    {
      $build=$1;
      print "Current build:$build\n";
      $build++;
      print "New build:$build\n";
      $_="  public static final int VER_BUILD = $build;\n";
    }
    if(/\s*public\s+static\s+final\s+String\s+VER_DATE\s*=\s*".*?";/)
    {
      $_='  public static final String VER_DATE = "'.localtime()."\";\n";
    }
  }
}else
{
  for(@l)
  {
    if(/#define\s+VER_BUILD\s+(\d+)/)
    {
      $build=$1;
      print "Current build:$build\n";
      $build++;
      print "New build:$build\n";
      $_="#define VER_BUILD $build\n";
    }
    if(/#define VER_DATE  ".*?"/)
    {
      $_='#define VER_DATE  "'.localtime()."\"\n";
    }
  }
}
open(F,'>'.$verfile) || die "Failed to open version.inc for writing";
print F @l;
close F;
system "cvs -q commit -m 'build $build' $verfile";
$mod=uc $mod;
my $tag=$mod.'_BUILD_'.$build;
print "Version tag: $tag\n";
$srcdir.='/' unless $srcdir=~m|/$|;
$srcdir.='../';
chdir $srcdir;
my $cmd="cvs tag $tag";
print "About to execute command:\n$cmd\n";
print "In dir:\n".cwd."\n";
print "Sure?(Y/N):";
my $yn=<STDIN>;
chomp $yn;
if(uc($yn) eq 'Y')
{
  system $cmd;
}
