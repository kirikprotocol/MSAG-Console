package CommaSeparated;
use strict;


my @OUT_FIELDS;
@OUT_FIELDS=(
{field=>'RECORD_TYPE',convert=>sub{return {'10'=>'08','20'=>'09'}->{$_[0]};},maxwidth=>2}, #1
{value=>''},                                                                               #2
{value=>''},                                                                               #3
{value=>''},                                                                               #4
{field=>'PAYER_IMSI',maxwidth=>15},                                                        #5
{field=>'PAYER_ADDR',maxwidth=>15},                                                        #5
{field=>'OTHER_ADDR',maxwidth=>15},                                                        #7
{value=>''},                                                                               #8
{value=>''},                                                                               #9
{value=>''},                                                                               #10
{value=>''},                                                                               #11
{value=>''},                                                                               #12
{value=>''},                                                                               #13
{value=>''},                                                                               #14
{value=>''},                                                                               #15
{value=>''},                                                                               #
{value=>''},                                                                               #16
{value=>''},                                                                               #17
{value=>''},                                                                               #18
{value=>''},                                                                               #19
{value=>''},                                                                               #20
{value=>''},                                                                               #21
{value=>''},                                                                               #22
{value=>''},                                                                               #23
{value=>''},                                                                               #24
{value=>''},                                                                               #25
{value=>''},                                                                               #26
{value=>''},                                                                               #27
{value=>''},                                                                               #28
{value=>''},                                                                               #29
{value=>''},                                                                               #30
{value=>''},                                                                               #31
{value=>''},                                                                               #32
{value=>''},                                                                               #33
{value=>''},                                                                               #34
{value=>''},                                                                               #35
{value=>''},                                                                               #36
{field=>'PAYER_MSC',maxwidth=>15},                                                         #37
{value=>''},                                                                               #
{value=>''},                                                                               #38
{value=>''},                                                                               #39
{value=>''},                                                                               #40
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
