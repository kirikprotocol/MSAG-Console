#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);

BEGIN{
  my $self=$0;
  if($self=~m/[\/\\]/)
  {
    $self=~s/[\/\\][^\/\\]+$//;
    print "path=$self\n";
    push @INC,$self;
  }
}

use vars qw($eoln);

use FixedWidth;
use CommaSeparated;

#use re 'debug';


my $header='';
my $footer='';
my $crc='0'x32;

$eoln="\x0d\x0a";

my $basedir=$0;
if($basedir=~/^(.*)[\\\/][^\\\/]+$/)
{
  $basedir=$1;
}else
{
  $basedir='.';
}

my $mscarr=[];

{
  open(F,"$basedir/filters.ini") or die "Failed to open $basedir/filters.ini:$!";
  my $h={};
  my $cursect;
  while(<F>)
  {
    s/[\x0d\x0a]//g;
    next if length($_)==0 || /^#/;
    if(/\[(.*)\]/)
    {
      $cursect=$1;
      next;
    }
    unless(defined($cursect))
    {
      die "Line out of section found in filters.ini";
    }
    $h->{$cursect}=[] unless exists $h->{$cursect};
    push @{$h->{$cursect}},$_;
  }
  close F;

  sub compilerx{
    my ($name,$arr)=@_;

    unless(@$arr)
    {
      die "Regexp $name is empty!";
    }
    my $str=join'|',map{"(?:$_)"}@$arr;
    #print "Compile $name:$str\n";
    my $rv;
    eval{
      $rv=qr($str);
    };
    if($@)
    {
      die "Compilation of regexp $name failed:$@";
    }
    return $rv;
  }
  for my $k(keys(%$h))
  {
    next unless $k=~/^(\w+).msc$/;
    my $rgn=$1;
    push @$mscarr,
    {
      mscrx=>compilerx("$rgn.msc",$h->{"$rgn.msc"}),
      addrrx=>compilerx("$rgn.addr",$h->{"$rgn.addr"}),
      imsirx=>compilerx("$rgn.imsi",$h->{"$rgn.imsi"})
    };
  }
}

my $svclstfile="$basedir/roamed_services.lst";

print "roamed_services:$svclstfile\n";

my $svcrx;

if(-e $svclstfile)
{
  open(F,$svclstfile);
  my @l=<F>;
  close F;
  s/[\x0d\x0a]//g for(@l);
  @l=grep !/^$/,@l;
  @l=grep !/^#/,@l;
  if(@l)
  {
    $svcrx='^(?:'.join('|',@l).')$';
    eval {
    $svcrx=qr($svcrx);
    };
    if($@)
    {
      print "roamed_services.lst contain invalid regexp pattern. Error is:$@\n";
      exit;
    }
  }
}
#print "svcrx=$svcrx\n";

my $regionsfile="$basedir/regions.lst";

print "Regions:$regionsfile\n";

my @regions;

if(-e $regionsfile)
{
  open(F,$regionsfile);
  my @l=<F>;
  close F;
  s/[\x0d\x0a]//g for(@l);
  @l=grep !/^$/,@l;
  @l=grep !/^#/,@l;
  if(@l)
  {
    for(@l)
    {
      if(/(\w+):(\w+):(.*)$/)
      {
        my $rx;
        eval{
          $rx=qr($3);
        };
        if($@)
        {
          print "Invalid regexp $3 in regions.lst\n";
          exit;
        }
        push @regions,{prefix=>$1,rx=>$rx,outpack=>$2};
      }else
      {
        print "Unrecognized line in regions.lst: $_\n";
        exit;
      }
    }
  }
}
if(!@regions)
{
  print "Required file: regions.lst not found or empty!\n";
  exit;
}

if($regions[-1]->{rx} ne qr(.*))
{
  print "Regexp of last line in regions.lst MUST be '.*' !\n";
  exit;
}


if(@ARGV!=4)
{
  print STDERR "usage: bill2cdr indir outdir tmpdir arcdir\n";
  exit;
}

my $indir=$ARGV[0];
my $outdir=$ARGV[1];
my $tmpdir=$ARGV[2];
my $arcdir=$ARGV[3];
for($indir,$outdir,$tmpdir,$arcdir)
{
  if(! -d $_)
  {
    die "error: $_ doesn't exists\n";
  }
}
for($indir,$outdir,$tmpdir,$arcdir)
{
  $_.='/' unless $_=~m!/$!;
}
opendir(D,$indir) or die "failed to read $indir";
my @dir=readdir(D);
closedir(D);

my @monthes=qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);

for(@dir)
{
  next unless $_=~/\.csv$/;
  my $infile=$indir.$_;
  my $ofn=$_;
  $ofn=~s/\.csv$/\.001/;
  $ofn=~s/_//;
  $ofn=~/(\d+)/;
  my $timestamp=$1;
  $timestamp=~/(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})/;
  my $t1=timegm($6,$5,$4,$3,$2-1,$1-1900);
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($t1);

  $year+=1900;
  my $curmon=$monthes[$mon];
  $mon++;
  $timestamp=sprintf("%04d%02d%02d%02d%02d%02d",$year,$mon,$mday,$hour,$min,$sec);

  my $tmpfile=[$tmpdir.$ofn];
  #my $outfile=$outdir.'ns'.$ofn;
  #print "$outfile\n";

  $header="90$timestamp".(' 'x81).'0'.(' 'x7).'90'.(' 'x17).'0'.(' 'x385);
  $footer="90$timestamp".(' 'x81)."0\x0d\x0a$crc";
  process($infile,$tmpfile);
  for my $tmp(@$tmpfile)
  {
    if(-f $tmp)
    {
      `chown smscdr:smsbill $tmp`;
      `chmod 664 $tmp`;
      $tmp=~/.*[\\\/]([^\\\/]+)$/;
      my $outfile=$outdir.$1;
      print "$outfile\n";
      if(!move($tmp,$outfile))
      {
        unlink $tmp;
        die "failed to move $tmp to $outfile";
      }
    }
  }
  my $arcout=sprintf("%s/%04d-%s",$arcdir,$year,$curmon);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  $arcout.=sprintf("/%02d",$mday);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  move($infile,$arcout);
}

sub outrow{
  my ($out,$fields)=@_;

  #return unless $fields->{PAYER_ADDR}=~/$addrrx/;
  #return unless $fields->{PAYER_MSC}=~/$mscrx/ || ($svcrx && $fields->{OTHER_ADDR}=~/$svcrx/);

  unless($svcrx && $fields->{OTHER_ADDR}=~/$svcrx/)
  {
    my $ok=0;
    for my $msc(@$mscarr)
    {
      #print "Match: msc: ".$fields->{PAYER_MSC}." =~ ".($msc->{mscrx})."\n";
      #print "Match: addr: ".$fields->{PAYER_ADDR}." =~ ".($msc->{addrrx})."\n";
      if
      (
        $fields->{PAYER_MSC}=~$msc->{mscrx} &&
        $fields->{PAYER_ADDR}=~$msc->{addrrx} &&
        $fields->{PAYER_IMSI}=~$msc->{imsirx}
      )
      {
        $ok=1;
        last;
      }
    }
    return unless $ok;
  }

  my $outf;
  my $outpack;
  for my $r(@regions)
  {
    #print "Check:".$fields->{PAYER_ADDR}." -> '".$r->{rx}."'\n";
    #my $str='9139292642'; #.$fields->{PAYER_ADDR};
    #my $rx=$r->{rx};
    #print "str='$str',rx='$rx'\n";
    #print 'ref='.ref($rx)."\n";
    #if( $str =~m/.*/ )
    if($fields->{PAYER_ADDR}=~$r->{rx})
    {
#      print "Match\n";
      $outf=$out->{$r->{prefix}};
      $outpack=$r->{outpack};
      last;
    }
  }
  if(!$outf)
  {
    print "Error: out file not found for payer ".$fields->{PAYER_ADDR}."\n";
    exit;
  }

  {
    no strict 'refs';
    "${outpack}::outrow"->($outf,$fields);
  }
  # TODO!!

}

sub conv_addr_other{
  my $addr=shift;
  if($addr=~/(\d)\.(\d)\.(\d+)/)
  {
#    return $3;
    if($1 eq '1')
    {
      $addr="+$3";
    }else
    {
      $addr=$3;
    };
  };
#  if($addr=~/\+(\d+)/)
#  {#
#    return $1;
#  }
  return $addr;
}


sub conv_addr_payer{
  my $addr=shift;

  if($addr=~/(\d)\.(\d)\.(\d+)/)
  {
    $addr=$3;
  };

  if($addr=~/\+(\d+)/)
  {
    $addr=$1;
  };

#  print substr($addr, 1);
  $addr = substr($addr, 1) if $addr=~/^7913/ || $addr=~/^83832/;
  if( $addr=~/913985(\d+)/ ) {
    $addr = "383213".$1;
  } elsif( $addr=~/913912(\d+)/ ) {
    $addr = "383214".$1;
  } elsif( $addr=~/913915(\d+)/ ) {
    $addr = "383291".$1;
  } elsif( $addr=~/913916(\d+)/ ) {
    $addr = "383292".$1;
  } elsif( $addr=~/913917(\d+)/ ) {
    $addr = "383299".$1;
  }
  return $addr;
}



sub datetotimestamp{
  my $date=shift;
  if($date=~/(\d+).(\d+).(\d+)\s+(\d+):(\d+):(\d+)/)
  {
    my $t1=timegm($6,$5,$4,$1,$2-1,$3-1900);
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($t1);

    $year+=1900;
    $mon++;
    return sprintf("%04d%02d%02d%02d%02d%02d",$year,$mon,$mday,$hour,$min,$sec);
  }else
  {
    die "Failed to parse input date:$date";
  };
};

sub process{
  my ($inf,$outarr)=@_;
  my $in=IO::File->new;
  if(!$in->open('<'.$inf)){die "Faield to open input file $inf"};
  my $out={};

  my $outtemplate=pop @$outarr;

  for my $r(@regions)
  {
    my $newfn=$outtemplate;
    $newfn=~s/(.*[\\\/])([^\\\/]+)$/$1.$r->{prefix}.$2/e;
    push @$outarr,$newfn;
    my $f=DelayedFile->new('>'.$newfn);
    $f->{header}=$header.$eoln;
    $f->{footer}=$footer.$eoln;
    $out->{$r->{prefix}}=$f;
#    print "Set file for prefix ".$r->{prefix}."\n";
  }
  my $csv=Text::CSV_XS->new({'binary'=>1});
  my $hdr=$csv->getline($in);
  die "Input file parsing failed" unless $hdr;
  my $row;
  while($row=$csv->getline($in))
  {
    last if @$row==0;
    my $infields={};
    $infields->{$hdr->[$_]}=$row->[$_]for(0..$#{$row});
    next if $infields->{STATUS}!=0;
    my $outfields={};
    %$outfields=%$infields;
    $outfields->{INV_SERVICE_ID}=$infields->{SERVICE_ID}==0?22:0;
    $outfields->{SERVICE_TYPE}='0';
    $outfields->{ACTION_CODE}='';
    if( $infields->{BEARER_TYPE} == 1 )
    {
      $outfields->{INV_SERVICE_ID}='ff';
      $outfields->{SERVICE_TYPE}='2';
      $outfields->{ACTION_CODE}='7';
      my $addrref;

#      print "BEARER_TYPE: ".$infields->{SRC_SME_ID}.','.$infields->{DST_SME_ID}."\n";

      if($infields->{SRC_SME_ID} ne 'MAP_PROXY')
      {
        $addrref=\$infields->{SRC_ADDR};
      }elsif($infields->{DST_SME_ID} ne 'MAP_PROXY')
      {
        $addrref=\$infields->{DST_ADDR};
      }

      if($addrref)
      {
        $$addrref=~s/^\.\d\.\d\.(ussd:)/$1/i;
        $$addrref=~s/^\.\d\.\d\.//i;
        $$addrref=~s/^(?!ussd:)/ussd:/i;
#        print "new addr:$$addrref\n";
      }
    }
    $outfields->{RECORD_TYPE}=10;
    $outfields->{CALL_DIRECTION}='O';
    $outfields->{PAYER_ADDR}=conv_addr_payer($infields->{SRC_ADDR});
    $outfields->{PAYER_IMSI}=$infields->{SRC_IMSI};
    $outfields->{PAYER_MSC}=$infields->{SRC_MSC};
#    $outfields->{OTHER_ADDR}=$infields->{DST_ADDR};
    $outfields->{OTHER_ADDR}=conv_addr_other($infields->{DST_ADDR});

    if($infields->{DST_ADDR}=~/^\.1\.1\./)
    {
      $outfields->{DD_TYPE}='05';
    }elsif($infields->{DST_ADDR}=~/^\.0\.1\.8/)
    {
      $outfields->{DD_TYPE}='06';
    }else
    {
      $outfields->{DD_TYPE}='';
    }

    $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
    outrow($out,$outfields);
    if($infields->{RECORD_TYPE}==1) # diverted sms
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      $outfields->{PAYER_ADDR}=conv_addr_payer($infields->{DIVERTED_FOR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr_other($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      $outfields->{INV_SERVICE_ID}=21;
      outrow($out,$outfields);
      $outfields->{RECORD_TYPE}=30;
      outrow($out,$outfields);
    }else
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      $outfields->{PAYER_ADDR}=conv_addr_payer($infields->{DST_ADDR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr_other($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      $outfields->{INV_SERVICE_ID}=21;
      outrow($out,$outfields);
    }
  }
}


=comment
'-1',
record_type,
call_direction,
DECODE(call_direction, 'I', TO_CHAR(finalized + 6 / 24,'yyyymmddhh24miss'),TO_CHAR(submit + 6 / 24, 'yyyymmddhh24miss')),
'1',
txt_length,
payer_imsi,
change_num (payer_addr),
'',
DECODE(other_ton, 1, '+', '') || other_addr,
'',
'',
'',
'',
'0',
'0',
'0',
DECODE(service_code, 0, 22, 0),
'',
service_code,
msg_id,
'0',
'0',
'0',
'0',
'0',
'',
'',
'0.000000',
'0',
'0'
=cut

package DelayedFile;
use IO::File;
use strict;

sub new{
  my $class=shift;
  $class=ref($class) if ref($class);
  my $fn=shift;
  die "Filename wasn't specified in constructor of DelayedFile" unless $fn;
  return bless {filename=>$fn},$class;
}

sub print{
  my $self=shift;
  unless($self->{opened})
  {
    $self->{handle}=IO::File->new;
    $self->{handle}->open($self->{filename}) || die "Failed to open ".$self->{filename};
    binmode($self->{handle});
    $self->{opened}=1;
    if($self->{header})
    {
      $self->{handle}->print($self->{header});
    }
  }
  $self->{handle}->print(@_);
}

sub close{
  my $self=shift;
  if($self->{opened})
  {
    $self->{handle}->close();
    $self->{opened}=0;
  }
}

sub DESTROY{
  my $self=shift;
  if($self->{opened})
  {
    if($self->{footer})
    {
      $self->print($self->{footer});
    }
    $self->{handle}=undef;
  }
}

1;
