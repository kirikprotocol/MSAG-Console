package FixedWidth;
use strict;

my @OUT_FIELDS;
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

sub outrow{
  my ($file,$fields)=@_;
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
    $file->print($v);
  }

  $file->print($main::eoln);
}

1;
