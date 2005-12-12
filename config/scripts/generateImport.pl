use strict;

use constant PROCLIMIT=>50;
use constant TIMEOUT=>60;

if(@ARGV!=2)
{
  print "Usage: $0 inputfile outputfile\n";
  exit;
}

open(my $f,$ARGV[0]) || die "Failed to open $ARGV[0]:$!";

my $smes={};

my $max=0;
while(<$f>)
{
  chomp;
  my ($sid,$pwd,$addr)=split(/\t/);
  if(!defined($sid) || !defined($pwd) || !defined($addr))
  {
    print STDERR "Invalid line $.:$_\n";
    next;
  }
  $max=length($sid) if length($sid)>$max;
  if(exists($smes->{$sid}))
  {
    if($smes->{$sid}->{pwd} ne $pwd)
    {
      print STDERR "Password doesn't match for sme $sid: $pwd!=@{[$smes->{$sid}->{pwd}]}!\n";
      next;
    }
    push @{$smes->{$sid}->{addrs}},$addr;
  }else
  {
    $smes->{$sid}=
    {
      pwd=>$pwd,
      addrs=>[$addr]
    }
  }
}
close $f;
print "maxsid=$max\n";

my @plmns=qw(
hmao.plmn
ynao.plmn
kurg.plmn
perm.plmn
tumn.plmn
ektb.plmn
chel.plmn
);

my @undo;

open($f,'>'.$ARGV[1]) || die "Failed to open $ARGV[1]:$!";
for my $sme(sort(keys(%$smes)))
{
  print $f qq[add sme $sme password @{[$smes->{$sme}->{pwd}]} timeout @{[TIMEOUT]} receiptScheme "default" proclimit @{[PROCLIMIT]}\n];
  push @undo,qq[delete sme $sme];
  print $f qq[add subject $sme notes "mar" $sme ];
  push @undo,qq[delete subject $sme];
  print $f join(',',map{qq[.0.1.$_]}@{$smes->{$sme}->{addrs}});
  print $f "\n";
  for my $plmn(@plmns)
  {
    print $f qq[add route "$sme > $plmn" notes "mar" active hide nobill arc allow receipt serviceid 0 priority 16000 src subj $sme dst subj $plmn MAP_PROXY\n];
    push @undo,qq[delete route "$sme > $plmn"];
    print $f qq[add route "$sme < $plmn" notes "mar" active hide   bill arc allow receipt serviceid 0 priority 16000 src subj $plmn dst subj $sme $sme\n];
    push @undo,qq[delete route "$sme < $plmn"];
  }
  print $f qq[alter route "ALL_SME_REPORTS < SMSC" add dst subj $sme $sme\n];
  push @undo,qq[alter route "ALL_SME_REPORTS < SMSC" delete dst subj $sme $sme];
}
print $f "apply routes";
close $f;

open($f,'>undo.con');
for(reverse(@undo))
{
  print $f "$_\n";
}
print $f "apply routes";
close $f;
