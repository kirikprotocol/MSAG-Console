#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);

#use re 'debug';

my @OUT_FIELDS;

my $header='';
my $footer='';
my $crc='0'x32;

my $eoln="\x0d\x0a";

my $addrrx=qr'7913\d{7}|913\d{7}|383213\d{4}|383214\d{4}|383291\d{4}|383292\d{4}|383299\d{4}';
my $mscrx=qr'79029860000|79139860001|79029869992';
my $imsirx=qr'25001390\d{7}|2500598\d{8}';

my $svclstfile=$0;
if($svclstfile=~/[\\\/]/)
{
  $svclstfile=~s/([\\\/])[^\\\/]+$/$1roamed_services.lst/;
}else
{
  $svclstfile='roamed_services.lst';
}

print "svc=$svclstfile\n";

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
print "svcrx=$svcrx\n";

my $regionsfile=$0;
if($regionsfile=~/[\\\/]/)
{
  $regionsfile=~s/([\\\/])[^\\\/]+$/$1regions.lst/;
}else
{
  $regionsfile='regions.lst';
}

print "Regions file:$regionsfile\n";

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
      if(/(\w+):(.*)$/)
      {
        my $rx;
        eval{
          $rx=qr($2);
        };
        if($@)
        {
          print "Invalid regexp $2 in regions.lst\n";
          exit;
        }
        push @regions,{prefix=>$1,rx=>$rx};
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


@OUT_FIELDS=(
{value=>'-1',width=>6},
{field=>'RECORD_TYPE',width=>5},    # pos=7
{field=>'CALL_DIRECTION',width=>5,align=>'R'},  # pos=12
{field=>'FINAL_DATE',width=>14},    # pos=17
{value=>'1',width=>10},       # pos=31
{field=>'DATA_LENGTH',width=>10},   # pos=41
{field=>'PAYER_IMSI',width=>21},    # pos=51
{field=>'PAYER_ADDR',width=>21},    # pos=72
{value=>'',width=>21},        # pos=93
{field=>'OTHER_ADDR',width=>31},    # pos=114
{value=>'',width=>31},        # pos=145
{value=>'',width=>17},        # pos=176
{value=>'',width=>10},        # pos=193
{value=>'',width=>10},        # pos=203
{value=>'0',width=>5},        # pos=213
{value=>'0',width=>10},       # pos=218
{field=>'SERVICE_TYPE',width=>5},   # pos=228 service type
{field=>'INV_SERVICE_ID',width=>10},    # pos=233 service id
{field=>'ACTION_CODE',width=>3},    # pos=243 action code
{value=>'0',width=>10},       # pos=246
{field=>'MSG_ID',width=>10,maxwidth=>10},      # pos=256
{value=>'0',width=>1},        # pos=266
{value=>'0',width=>1},        # pos=267
{value=>'0',width=>1},        # pos=268
{value=>'0',width=>1},        # pos=269
{value=>'0',width=>1},        # pos=270
{value=>'',width=>1},       # pos=271
{value=>'',width=>20},        # pos=272
{value=>'0.000000',width=>28},      # pos=292
{value=>'0',width=>10},       # pos=320
{value=>'0',width=>10},       # pos=330
);

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

  return unless $fields->{PAYER_ADDR}=~/$addrrx/;
  return unless $fields->{PAYER_MSC}=~/$mscrx/ || ($svcrx && $fields->{OTHER_ADDR}=~/$svcrx/);
  return unless $fields->{PAYER_IMSI}=~/$imsirx/;

  my $outf;
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
      last;
    }
  }
  if(!$outf)
  {
    print "Error: out file not found for payer ".$fields->{PAYER_ADDR}."\n";
    exit;
  }

  for my $f(@OUT_FIELDS)
  {
    my $v;
    if(exists($f->{field}))
    {
      $v=$fields->{$f->{field}};
    }else
    {
      $v=$f->{value};
    }
    $v=substr($v,-$f->{maxwidth}) if length($v)>$f->{maxwidth};
    if($f->{align} eq 'R')
    {
      $v=' 'x($f->{width}-length($v)).$v;
    }else
    {
      $v.=' 'x($f->{width}-length($v));
    }
    $outf->print($v);
  }

  $outf->print($eoln);
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
    $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT});
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
