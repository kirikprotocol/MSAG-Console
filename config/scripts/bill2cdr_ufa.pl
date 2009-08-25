#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);
use Fcntl ':flock';

my $f;
open($f,'>>/global/data/conf/scripts/bill_lock') || die "Failed to open lock file:$!";
flock($f,LOCK_EX) || die "Lock failed:$!";

my @OUT_FIELDS;

my $header='';
my $footer='';
my $crc='0'x32;

my $addrrx=qr'79\d{9}';
my $mscrx=qr'.*';

my $eoln="\x0d\x0a";

@OUT_FIELDS=(
{value=>'-1',width=>6},                          #01   1
{field=>'RECORD_TYPE',width=>5},                 #02   7
{field=>'CALL_DIRECTION',width=>5,align=>'R'},   #03  12
{field=>'FINAL_DATE',width=>14},                 #04  17
{field=>'PARTS_NUM',width=>10},                  #05  31
{field=>'DATA_LENGTH',width=>10},                #06  41
{field=>'PAYER_IMSI',width=>21},                 #07  51
{field=>'PAYER_ADDR',width=>21},                 #08  72
{value=>'',width=>21},                           #09  93
{field=>'OTHER_ADDR',width=>31},                 #10 114
{value=>'',width=>31},                           #11 145
{value=>'',width=>17},                           #12 176
{value=>'',width=>10},                           #13 193
{value=>'',width=>10},                           #14 203
{value=>'0',width=>5},                           #15 213
{value=>'0',width=>10},                          #16 218
{value=>'0',width=>5},                           #17 228
{field=>'INV_SERVICE_ID',width=>10},             #18 233
{value=>'',width=>3},                            #19 243
{value=>'0',width=>10},                          #20 246
{value=>'0',width=>10},                          #21 256
#{field=>'SERVICE_ID',width=>10},
#{field=>'MSG_ID',width=>10},
{value=>'0',width=>1},                           #22 266
{value=>'0',width=>1},                           #23 267
{value=>'0',width=>1},                           #24 268
{value=>'0',width=>1},                           #25 269
{value=>'0',width=>10,align=>'R'},               #26 270
{value=>'0.000000',width=>20,align=>'R'},        #27 280
{field=>'PAYER_MSC',width=>20},                  #28 300
{field=>'MSG_ID',width=>10,maxlength=>10},       #29 320
{value=>'0',width=>10},                          #30 330
);

if(@ARGV<4)
{
  print STDERR "usage: bill2cdr indir outdir tmpdir arcdir [nobilldir]\n";
  exit;
}

my $indir=$ARGV[0];
my $outdir=$ARGV[1];
my $tmpdir=$ARGV[2];
my $arcdir=$ARGV[3];
my $nobilldir=$ARGV[4];
for($indir,$outdir,$tmpdir,$arcdir,$nobilldir)
{
  if($_ && ! -d $_)
  {
    die "error: $_ doesn't exists\n";
  }
}
for($indir,$outdir,$tmpdir,$arcdir,$nobilldir)
{
  next unless $_;
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

  my $tmpfile=$tmpdir.$ofn;
  my $outfile=$outdir.
              sprintf("SMS2_%02d%s%d_%02d&%02d&%02d_",$mday,$curmon,$year,$hour,$min,$sec).
              $ofn;
  print "$outfile\n";

  $header="90$timestamp".(' 'x81).'0'.(' 'x7).'90'.(' 'x17).'0'.(' 'x385);
  $footer="90$timestamp".(' 'x81)."0$eoln$crc";
  eval{
    process($infile,$tmpfile);
  };
  if($@)
  {
    print STDERR "Processing error on file $infile:'$@'\n";
    rename($infile,$infile.'.err');
  }

  if(-f $tmpfile)
  {
    if(!move($tmpfile,$outfile))
    {
      unlink $tmpfile;
      die "failed to move $tmpfile to $outfile";
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
  $arcout.=sprintf("/%02d",$hour);
  unless(-d $arcout)
  {
    mkdir $arcout,0755;
  }
  move($infile,$arcout);
}

sub outrow{
  my ($out,$fields)=@_;

  return 0 unless $fields->{PAYER_ADDR}=~/$addrrx/;
  return 0 unless $fields->{PAYER_MSC}=~/$mscrx/;

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
    $v=substr($v,-$f->{maxlength}) if length($v)>$f->{maxlength};

    if($f->{align} eq 'R')
    {
      $v=' 'x($f->{width}-length($v)).$v;
    }else
    {
      $v.=' 'x($f->{width}-length($v));
    }
    $out->print($v);
  }

  $out->print($eoln);
  return 1;
}

sub conv_addr{
  my $addr=shift;
  if($addr=~/(\d)\.(\d)\.(\d+)/)
  {
    return $3;
#    if($1 eq '1')
#    {
#      $addr="+$3";
#    }else
#    {
#      $addr=$3;
#    }
  }
  if($addr=~/\+(\d+)/)
  {
    return $1;
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

sub getFileName{
  my $path=shift;
  if($path=~m!.*/([^/]+)!)
  {
    return $1;
  }
  return $path;
}

sub combine{
  my ($csv,$arr)=@_;
  $csv->combine(@$arr);
  return $csv->string().$eoln;
}

sub process{
  my ($inf,$outf)=@_;
  print "Processing $inf\n";
  my $in=IO::File->new;
  if(!$in->open('<'.$inf)){die "Faield to open input file $inf"};
  my $out=DelayedFile->new('>'.$outf);
  $out->{header}=$header.$eoln;
  $out->{footer}=$footer.$eoln;
  my $csv=Text::CSV_XS->new({'binary'=>1});
  my $hdr=$csv->getline($in);
  my $nbout=$nobilldir?DelayedFile->new('>'.$nobilldir.getFileName($inf)):undef;
  $nbout->{header}=combine($csv,$hdr) if $nbout;
  die "Input file parsing failed" unless $hdr;
  my $row;
  while($row=$csv->getline($in))
  {
    last if @$row==0;
    my $infields={};
    $infields->{$hdr->[$_]}=$row->[$_]for(0..$#{$row});
    next if $infields->{STATUS}!=0;
    my $outfields={};
    $infields->{MSG_ID}%=0x100000000;
    %$outfields=%$infields;
    $outfields->{INV_SERVICE_ID}=$infields->{SERVICE_ID}==0?22:$infields->{SERVICE_ID};

    if( $infields->{BEARER_TYPE} == 1 )
    {
      $outfields->{INV_SERVICE_ID}='98';
      my $addrref;


      if($infields->{SRC_SME_ID} ne 'MAP_PROXY')
      {
        $addrref=\$infields->{SRC_ADDR};
      }elsif($infields->{DST_SME_ID} ne 'MAP_PROXY')
      {
        $addrref=\$infields->{DST_ADDR};
      }

      if($addrref)
      {
        $$addrref=~s/^\.\d\.\d\.ussd://i;
        $$addrref=~s/^\.\d\.\d\.//i;
        #$$addrref=~s/^(?!ussd:)/ussd:/i;
      }
    }

    $outfields->{RECORD_TYPE}=10;
    $outfields->{CALL_DIRECTION}='O';
    $outfields->{PAYER_ADDR}=conv_addr($infields->{SRC_ADDR});
    $outfields->{PAYER_IMSI}=$infields->{SRC_IMSI};
    $outfields->{PAYER_MSC}=$infields->{SRC_MSC};
    $outfields->{OTHER_ADDR}=conv_addr($infields->{DST_ADDR});
    $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT});
    unless(outrow($out,$outfields))
    {
      $nbout->print(combine($csv,$row)) if $nbout;
    }
    if($infields->{RECORD_TYPE}==1) # diverted sms
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      $outfields->{PAYER_ADDR}=conv_addr($infields->{DIVERTED_FOR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      unless(outrow($out,$outfields))
      {
        $nbout->print(combine($csv,$row)) if $nbout;
      }
      $outfields->{RECORD_TYPE}=30;
      unless(outrow($out,$outfields))
      {
        $nbout->print(combine($csv,$row)) if $nbout;
      }
    }else
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      if( $infields->{BEARER_TYPE} == 1 ) # ussd
      {
        $outfields->{INV_SERVICE_ID}='97';
      }
      if( $infields->{BEARER_TYPE} == 0 ) # sms
      {
        $outfields->{INV_SERVICE_ID}='21';
      }
      $outfields->{PAYER_ADDR}=conv_addr($infields->{DST_ADDR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      unless(outrow($out,$outfields))
      {
        $nbout->print(combine($csv,$row)) if $nbout;
      }
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
    $self->{opened}=1;
    binmode($self->{handle});
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
