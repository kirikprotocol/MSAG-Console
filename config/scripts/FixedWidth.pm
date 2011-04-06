package FixedWidth;
use strict;

my @OUT_FIELDS;
@OUT_FIELDS=(
{value=>'-1',width=>6},
{field=>'RECORD_TYPE',width=>5},                # pos=7
{field=>'CALL_DIRECTION',width=>5,align=>'R'},  # pos=12
{field=>'FINAL_DATE',width=>14},                # pos=17
{value=>'1',width=>10},                         # pos=31
{field=>'DATA_LENGTH',width=>10},               # pos=41
{field=>'PAYER_IMSI',width=>21},                # pos=51
{field=>'PAYER_ADDR',width=>21},                # pos=72
{field=>'CBOSSCDR_ID3',width=>21},              # pos=93
{field=>'OTHER_ADDR',width=>31},                # pos=114
{value=>'',width=>31},                          # pos=145
{value=>'',width=>17},                          # pos=176
{field=>'SRC_SME_ID',width=>10},                # pos=193 inc route
{field=>'DST_SME_ID',width=>10},                # pos=203 out route
{value=>'0',width=>5},                          # pos=213
{value=>'0',width=>10},                         # pos=218
{field=>'SERVICE_TYPE',width=>5},               # pos=228 service type
{field=>'INV_SERVICE_ID',width=>10},            # pos=233 service id
{field=>'ACTION_CODE',width=>3},                # pos=243 action code
{value=>'0',width=>10},                         # pos=246
{field=>'MSG_ID',width=>10,maxwidth=>10},       # pos=256
{value=>'0',width=>1},                          # pos=266
{field=>'IN_BILL',value=>'0',width=>1},         # pos=267
{value=>'0',width=>1},                          # pos=268
{value=>'0',width=>1},                          # pos=269
{value=>'0',width=>10},                         # pos=270
{value=>'0.000000',width=>20,align=>'R'},       # pos=280
{field=>'PAYER_MSC',width=>20},                 # pos=300
{value=>'0',width=>10},                         # pos=320
{value=>'0',width=>10},                         # pos=330
);

sub outrow{
  my ($file,$fields)=@_;
  for my $f(@OUT_FIELDS)
  {
    my $v;
    if(exists($f->{field}) && exists($fields->{$f->{field}}))
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
    $file->print($v);
  }

  $file->print($main::eoln);
}

1;
