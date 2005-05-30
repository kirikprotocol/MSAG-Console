package CommaSeparated2;
use strict;


my @OUT_FIELDS;
@OUT_FIELDS=(
{field=>'RECORD_TYPE',convert=>sub{return {'10'=>'08','20'=>'09'}->{$_[0]};},maxwidth=>2}, #1  REC_TYPE
{value=>'0'},                                                                              #2  REC_NUMBER
{value=>''},                                                                               #3  REC_STATUS
{value=>''},                                                                               #4  REC_SUB_NUMBER
{field=>'PAYER_IMSI',maxwidth=>15},                                                        #5  IMSI
{field=>'PAYER_ADDR',maxwidth=>15},                                                        #6  MSISDN
{field=>'OTHER_ADDR',maxwidth=>15,convert=>sub{$_[0]=~/\+?(.*)$/;return $1;}},             #7  DIALED
{field=>'FINAL_DATE',maxwidth=>14},                                                        #8  START_TIME
{value=>''},                                                                               #9  DURATION
{value=>''},                                                                               #10 SUCCESS
{field=>'DD_TYPE',maxwidth=>2},                                                            #11 DD_TYPE
{value=>''},                                                                               #12 CALL_TYPE
{value=>''},                                                                               #13 A_FORWARD
{value=>''},                                                                               #14 IMEI
{value=>''},                                                                               #15 MS_CLASS
{value=>''},                                                                               #   TYPE_1_SER
{value=>''},                                                                               #16 CODE_1_SER
{value=>''},                                                                               #17 TYPE_2_SER
{value=>''},                                                                               #18 CODE_2_SER
{value=>'0'},                                                                              #19 A_AREA
{value=>'0'},                                                                              #20 A_CELL
{value=>'0'},                                                                              #21 B_AREA
{value=>'0'},                                                                              #22 B_CELL
{value=>''},                                                                               #23 ACTION
{value=>''},                                                                               #24 SERV_CODE
{value=>''},                                                                               #25 REASON
{field=>'OTHER_ADDR',maxwidth=>24},                                                        #26 CALLED_NUMBER
{value=>''},                                                                               #27 Reserved
{value=>''},                                                                               #28 Reserved
{value=>''},                                                                               #29 Reserved
{value=>''},                                                                               #30 CIRCUIT_IN
{value=>''},                                                                               #31 CIRCUIT_OUT
{value=>''},                                                                               #32 Reserved
{value=>''},                                                                               #33 Reserved
{value=>''},                                                                               #34 Reserved
{value=>''},                                                                               #35 Reserved
{value=>''},                                                                               #36 Reserved
{field=>'PAYER_MSC',maxwidth=>15},                                                         #37 MSC_ID
{value=>''},                                                                               #   TRAFIC_PRICE_$
{value=>''},                                                                               #38 TRAFIC_BY_DIR_ PRICE_$
{value=>''},                                                                               #39 IN_BALANCE_S
{value=>''},                                                                               #40 UDF
);

sub outrow{
  my ($file,$fields)=@_;
  return if $fields->{RECORD_TYPE}==30;
  $file->{header}='';
  $file->{footer}='';
  my $first=1;
  for my $f(@OUT_FIELDS)
  {
    $file->print(',') unless$first;
    $first=0;

    my $v;
    if(exists($f->{field}))
    {
      $v=$fields->{$f->{field}};
    }else
    {
      $v=$f->{value};
    }
    $v=$f->{convert}->($v) if exists $f->{convert};
    $v=substr($v,-$f->{maxwidth}) if(exists($f->{maxwidth}) && length($v)>$f->{maxwidth});
    $file->print($v);
  }

  $file->print($main::eoln);
}

1;
