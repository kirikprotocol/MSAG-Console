#!/usr/bin/perl

use Text::CSV_XS;
use IO::File;
use strict;
use File::Copy;
use Time::Local qw(timegm timelocal);

my @OUT_FIELDS;

my $header='';
my $footer='';
my $crc='0'x32;

my $addrrx=qr'7913\d{7}';
my $mscrx=qr'79029860000|79139860001|79029869992';

my $eoln="\n";

@OUT_FIELDS=(
{value=>'-1',width=>6},
{field=>'RECORD_TYPE',width=>5},
{field=>'CALL_DIRECTION',width=>5,align=>'R'},
{field=>'FINAL_DATE',width=>5},
{value=>'1',width=>10},
{field=>'DATA_LENGTH',width=>10},
{field=>'PAYER_IMSI',width=>21},
{field=>'PAYER_ADDR',width=>21},
{value=>'',width=>21},
{field=>'OTHER_ADDR',width=>31},
{value=>'',width=>31},
{value=>'',width=>17},
{value=>'',width=>10},
{value=>'',width=>10},
{value=>'0',width=>5},
{value=>'0',width=>10},
{value=>'0',width=>5},
{field=>'INV_SERVICE_ID',width=>10},
{value=>'',width=>3},
{value=>'0',width=>10},
{value=>'0',width=>10},
#{field=>'SERVICE_ID',width=>10},
#{field=>'MSG_ID',width=>10},
{value=>'0',width=>1},
{value=>'0',width=>1},
{value=>'0',width=>1},
{value=>'0',width=>1},
{value=>'0',width=>1},
{value=>'',width=>1},
{value=>'',width=>20},
{value=>'0.000000',width=>28},
#{value=>'0',width=>10},
{field=>'MSG_ID',width=>10},
{value=>'0',width=>10},
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
for(@dir)
{
  next unless $_=~/\.csv$/;
  my $infile=$indir.$_;
  my $ofn=$_;
  $ofn=~s/\.csv$/\.cdr/;
  $ofn=~s/_//;
  $ofn=~/(\d+)/;
  my $timestamp=$1;
  {
    $timestamp=~/(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})/;
    my $t1=timegm($6,$5,$4,$3,$2-1,$1-1900);
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($t1);

    $year+=1900;
    $mon++;
    $timestamp="$year$mon$mday$hour$min$sec";
  }
  my $tmpfile=$tmpdir.$ofn;
  my $outfile=$outdir.$ofn;
  $header="90$timestamp".(' 'x81).'0'.(' 'x7).'90'.(' 'x17).'0'.(' 'x385);
  $footer="90$timestamp".(' 'x81)."0\n$crc";
  process($infile,$tmpfile);
  if(!move($tmpfile,$outfile))
  {
    unlink $tmpfile;
    die "failed to move $tmpfile to $outfile";
  }
  move($infile,$arcdir);
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
    if($f->{align} eq 'R')
    {
      $v=' 'x($f->{width}-length($v)).$v;
    }else
    {
      $v.=' 'x($f->{width}-length($v));
    }
    print $out $v;
  }

  print $out $eoln;
}

sub conv_addr{
  my $addr=shift;
  if($addr=~/(\d)\.(\d)\.(\d+)/)
  {
    if($1 eq '1')
    {
      $addr="+$3";
    }else
    {
      $addr=$3;
    }
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
    return "$year$mon$mday$hour$min$sec";
  }else
  {
    die "Failed to parse input date:$date";
  };
};

sub process{
  my ($inf,$outf)=@_;
  my $in=IO::File->new;
  if(!$in->open('<'.$inf)){die "Faield to open input file $inf"};
  my $out=IO::File->new;
  if(!$out->open('> '.$outf)){die "Faield to open output file $outf"};
  print $out "$header\n";
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

    $outfields->{RECORD_TYPE}=10;
    $outfields->{CALL_DIRECTION}='O';
    $outfields->{PAYER_ADDR}=conv_addr($infields->{SRC_ADDR});
    $outfields->{PAYER_IMSI}=$infields->{SRC_IMSI};
    $outfields->{PAYER_MSC}=$infields->{SRC_MSC};
    $outfields->{OTHER_ADDR}=conv_addr($infields->{DST_ADDR});
    $outfields->{FINAL_DATE}=datetotimestamp($infields->{SUBMIT});
    outrow($out,$outfields);
    if($infields->{RECORD_TYPE}==1) # diverted sms
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      $outfields->{PAYER_ADDR}=conv_addr($infields->{DIVERTED_FOR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      outrow($out,$outfields);
      $outfields->{RECORD_TYPE}=30;
      outrow($out,$outfields);
    }else
    {
      $outfields->{RECORD_TYPE}=20;
      $outfields->{CALL_DIRECTION}='I';
      $outfields->{PAYER_ADDR}=conv_addr($infields->{DST_ADDR});
      $outfields->{PAYER_IMSI}=$infields->{DST_IMSI};
      $outfields->{PAYER_MSC}=$infields->{DST_MSC};
      $outfields->{OTHER_ADDR}=conv_addr($infields->{SRC_ADDR});
      $outfields->{FINAL_DATE}=datetotimestamp($infields->{FINALIZED});
      outrow($out,$outfields);
    }
  }
  print $out "$footer\n";
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
