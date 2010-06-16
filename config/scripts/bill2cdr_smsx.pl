#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);
use Fcntl ':flock';

use constant EXTRAMSC => '79168960220';

use constant POSTPAID=>1;
use constant PREPAID=>2;

use constant CHARGE_SUBMIT=>0;
use constant CHARGE_DELIVERY=>1;
use constant CHARGE_DATACOLLECTED=>2;
use constant CHARGE_SUBMIT_COLLECTED=>3;

my $f;
open($f,'>>/data/conf/scripts/lock') || die "Failed to open lock file:$!";
flock($f,LOCK_EX) || die "Lock failed:$!";

my @OUT_FIELDS;

my $header='';
my $footer='';
my $crc='0'x32;

my $addrrx=qr'79\d{9}';
my $mscrx=qr'.*';

my $eoln="\x0d\x0a";

my %EXTRA_MAPPING=(
  1=>'00007961',
  2=>'00007962',
  3=>'00007963',
  4=>'00007964',
  5=>'00007965',
  6=>'00007965',
  7=>'00007965',
  8=>'00007965'
);

@OUT_FIELDS=(
{value=>'-1',width=>6},                          #01   1 bIsFirstLeg
{field=>'RECORD_TYPE',width=>5},                 #02   7 Record Type 10-in, 20-out, 30-divert
{field=>'CALL_DIRECTION',width=>5,align=>'R'},   #03  12 Call Direction I/O
{field=>'FINAL_DATE',width=>14},                 #04  17 Date & Time
{field=>'CALL_DURATION',width=>10},                  #05  31 Call Duration or SMS parts num
{field=>'DATA_LENGTH',width=>10},                #06  41 Data Volume (size)
{field=>'PAYER_IMSI',width=>21},                 #07  51 ID
{field=>'PAYER_ADDR',width=>21},                 #08  72 ID2
{field=>'SMSX_SRV',width=>21},                   #09  93 ID3
{field=>'OTHER_ADDR',width=>31},                 #10 114 Other Party
{value=>'',width=>31},                           #11 145 Fwd A, diverted adddress
{value=>'',width=>17},                           #12 176 IMEI
{value=>'',width=>10},                           #13 193 Incoming Route
{value=>'',width=>10},                           #14 203 Outgoing Route
{value=>'0',width=>5},                           #15 213 Location Area
{value=>'0',width=>10},                          #16 218 Cell
{value=>'0',width=>5},                           #17 228 OL Service Type
{field=>'INV_SERVICE_ID',width=>10},             #18 233 OL Service Id
{value=>'',width=>3},                            #19 243 OL Service Extension
{value=>'0',width=>10},                          #20 246 Service Code
{value=>'0',width=>10},                          #21 256 Chain Reference
{value=>'0',width=>1},                           #22 266 Is Forwarded
{field=>'ISPRECHARGED',width=>1},                #23 267 IsPreCharged
{value=>'0',width=>1},                           #24 268 IsHasTax
{value=>'0',width=>1},                           #25 269 IsFirstCdr
{value=>'0',width=>10,align=>'R'},               #26 270 Company Zone
{value=>'0.000000',width=>20,align=>'R'},        #27 280 Charge
{field=>'PAYER_MSC',width=>20},                  #28 300 MscId
{field=>'MSG_ID',width=>10,maxlength=>10},       #29 320 SeqN
{value=>'0',width=>10},                          #30 330 OriginNetworkID
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

  my $tmpfile=$tmpdir.$ofn;
  my $outfile=$outdir.
              sprintf("smsx_%04d%02d%02d_%02d%02d%02d_",$year,$mon,$mday,$hour,$min,$sec).
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
    `chown cdr:cdr $tmpfile`;
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

  return unless $fields->{PAYER_ADDR}=~/$addrrx/;
  return unless $fields->{PAYER_MSC}=~/$mscrx/;

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
}

sub conv_addr{
  my $addr=shift;
  if($addr=~/^\.(\d)\.(\d)\.(.+)$/)
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
  if($addr=~/^\+(.+)$/)
  {
    return $1;
  }
  return $addr;
}

sub datetotimestamp{
  my $date=shift;
  my $offset=shift;
  if($date=~/(\d+).(\d+).(\d+)\s+(\d+):(\d+):(\d+)/)
  {
    my $t1=timegm($6,$5,$4,$1,$2-1,$3-1900);
    $t1+=$offset;
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
  my ($inf,$outf)=@_;
  print "Processing $inf\n";
  my $in=IO::File->new;
  if(!$in->open('<'.$inf)){die "Faield to open input file $inf"};
  my $out=DelayedFile->new('>'.$outf);
  $out->{header}=$header.$eoln;
  $out->{footer}=$footer.$eoln;
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

    my $ihsuffix='';
    #changed by request (paid websms/webgroups support) 11.09.2008
    #changed by request (paid websms/webgroups support) 07.11.2008
    #if(($infields->{SRC_SME_ID} eq 'webgroup' || $infields->{SRC_SME_ID} eq 'websms') && $infields->{SRC_MSC} ne '')
    if($infields->{SRC_MSC} eq 'GT')
    {
#      $infields->{DST_ADDR}=$infields->{DST_ADDR}.'IH';
      $ihsuffix='IH';
    }

    #changed by request 19.10.2007
    #changed for webgroups/websms 07.11.2008
    if(($infields->{SRC_SME_ID} ne 'MAP_PROXY' && $infields->{SRC_MSC} eq '') || $infields->{SRC_MSC} eq 'GT')
    {
      $infields->{SRC_MSC}=EXTRAMSC;
    }
    if(($infields->{DST_SME_ID} ne 'MAP_PROXY' && $infields->{DST_MSC} eq '') || $infields->{DST_MSC} eq 'GT')
    {
      $infields->{DST_MSC}=EXTRAMSC;
    }

    $outfields->{CALL_DURATION}=1;
    #$outfields->{CALL_DURATION}=$infields->{PARTS_NUM};
    #changed by request of MTC at 27.06.2007

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


    #by request of Igor G at 7.4.2008
    #$outfields->{SRC_ADDR}=~s/^\.5\.0\.//;
    #$outfields->{DST_ADDR}=~s/^\.5\.0\.//;

    $outfields->{ISPRECHARGED}=0;
    if($infields->{SMSX_SRV}&0x80000000)
    {
      $outfields->{SMSX_SRV}=$infields->{SMSX_SRV}&~0x80000000;
      $outfields->{ISPRECHARGED}=1;
    }

    my $makeOutRec=1;
    my $makeInRec=1;

    my $extraOut;

    if(exists($EXTRA_MAPPING{int($infields->{SMSX_SRV})}))
    {
      $extraOut=$EXTRA_MAPPING{int($infields->{SMSX_SRV})};
    }

    if($infields->{CONTRACT}==PREPAID && $infields->{SMSX_SRV}!=4 && $infields->{SMSX_SRV}!=7 && $infields->{SMSX_SRV}!=8)
    {
      $makeOutRec=0;
    }

    if($infields->{CHARGE}==CHARGE_DATACOLLECTED || $infields->{CHARGE}==CHARGE_SUBMIT_COLLECTED)
    {
      $makeOutRec=0;
      $extraOut=undef;
    }elsif($infields->{CHARGE}==CHARGE_SUBMIT)
    {
      $makeInRec=0;
    }

    if($infields->{DST_SME_ID} eq 'smsx')
    {
      $makeInRec=0;
    }



    $outfields->{RECORD_TYPE}=10;
    $outfields->{CALL_DIRECTION}='O';
    $outfields->{INV_SERVICE_ID}=22;
    $outfields->{PAYER_ADDR}=conv_addr($infields->{SRC_ADDR});
    $outfields->{PAYER_IMSI}=$infields->{SRC_IMSI};
    $outfields->{PAYER_MSC}=$infields->{SRC_MSC};
    $outfields->{OTHER_ADDR}=conv_addr($infields->{DST_ADDR}).$ihsuffix;
    if($makeOutRec)
    {
      for my $off(1 .. $outfields->{PARTS_NUM})
      {
        $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT},$off);
        outrow($out,$outfields);
      }

    }
    if(defined($extraOut))
    {
      $outfields->{OTHER_ADDR}=$extraOut;
      for my $off(1 .. $outfields->{PARTS_NUM})
      {
        $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT},$off);
        outrow($out,$outfields);
      }
    }


    if($makeInRec)
    {
      if($infields->{RECORD_TYPE}==1) # diverted sms
      {
        $outfields->{RECORD_TYPE}=20;
        $outfields->{CALL_DIRECTION}='I';
        $outfields->{INV_SERVICE_ID}=21;
        $outfields->{PAYER_ADDR}=conv_addr($infields->{DIVERTED_FOR});
        $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
        $outfields->{PAYER_MSC}=$infields->{DST_MSC};
        $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR}).$ihsuffix;
        for my $off(1 .. $outfields->{PARTS_NUM})
        {
          $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT},$off);
          outrow($out,$outfields);
        }
        $outfields->{RECORD_TYPE}=30;
        for my $off(1 .. $outfields->{PARTS_NUM})
        {
          $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT},$off);
          outrow($out,$outfields);
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
        $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR}).$ihsuffix;
        for my $off(1 .. $outfields->{PARTS_NUM})
        {
          $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT},$off);
          outrow($out,$outfields);
        }
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
