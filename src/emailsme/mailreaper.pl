#! /usr/bin/perl
use strict;
use MIME::Parser;
use Lingua::DetectCharset;
use Convert::Cyrillic;

binmode(STDOUT);
binmode(STDIN);

$|=1;

my $logfh;
my $log=1;
sub Log{
  return unless $log;
  if(!$logfh)
  {
    if(open($logfh,'>>maillog'))
    {
      print $logfh "=== log started ===\n";
      select($logfh);
      $|=1;
      select(STDOUT);
    }else
    {
      $logfh=undef;
      $log=undef;
      return;
    }
  }
  print $logfh @_,"\n";
}

Log("Starting...");

while(my $sz=<STDIN>)
{
  Log("Received:$sz");
  my $raw;
  read(STDIN,$raw,$sz);
  Log("Read:$raw");
  my $parser=new MIME::Parser;
  $parser->decode_headers(1);
  $parser->output_to_core(1);#'tmp');
  $parser->tmp_to_core(1);
  my $entity=eval{$parser->parse_data($raw);};
  if($@)
  {
    Log("Error:$@");
    exit;
  }
  my $head=$entity->head;
  my $subj=$head->get('Subject');
  my $from=$head->get('From');

  my $msg='';
  my $msgct='none';

  if($entity->is_multipart)
  {
    sub Check{
      my ($e,$msgref,$msgctref)=@_;
      for my $num(0..$e->parts-1)
      {
        my $part=$e->parts($num);
        if($part->head->get('Content-Type') =~ m'text/plain')
        {
          $$msgref=$part->bodyhandle->as_string;
          $$msgctref='text';
          last;
        }
        if($part->head->get('Content-Type') =~ m'text/html')
        {
          $$msgref=$part->bodyhandle->as_string;
          $$msgctref='html';
        }
        if($part->is_multipart)
        {
          Check($part,$msgref,$msgctref);
        }
      }
    }
    Check($entity,\$msg,\$msgct);
  }else
  {
    $msg=$entity->bodyhandle->as_string;
    if($entity->head->get('Content-Type')=~m'text/html')
    {
      $msgct='html';
    }else
    {
      $msgct='text';
    }
  }

  if($msgct eq 'html')
  {
    require HTML::Parser;
    my @text;
    my $p=HTML::Parser->new(handlers=>
      {
        text=>[\@text,'event,text'],
        start=>[\@text,'event,tagname'],
        end=>[\@text,'event,tagname'],
      });
    $p->unbroken_text(1);
    #$p->report_tags('br','div');
    $p->parse($msg);
    $p->eof;
    $msg='';
    my $skip=0;
    for(@text)
    {
      if($_->[0] eq 'text')
      {
        $msg.=$_->[1] unless $skip;
      }elsif($_->[0] eq 'start')
      {
        Log('<'.$_->[1].">");
        $msg.="\x00" if(lc($_->[1]) eq 'br');
        $skip=$skip || {'script'=>1,'style'=>1}-> { lc($_->[1]) };
      }elsif($_->[0] eq 'end')
      {
        Log('</'.$_->[1].">");
        $msg.="\x00" if(lc($_->[1]) eq 'div');
        $skip=0 if {'script'=>1,'style'=>1,'table'=>1}-> { lc($_->[1]) };
      }
    }
    $msg=~s/\x0d//g;
    $msg=~s/([^\s\x00])\x0a([^\s\x00])/$1 $2/g;
    $msg=~s/\x0a//g;
    $msg=~s/\s+/ /g;
    $msg=~s/\x00/\n/g;
    $msg=~s/&nbsp;/ /gi;
    $msg=~s/&(?!amp|apos).*?;//gi;
    $msg=~s/&amp;/&/gi;
    $msg=~s/&apos;/'/gi;
    $msg=~s/^  +$//gm;
    $msg=~s/ +$//gm;
  }

  $msg=~s/\n+/\n/g;

  $msg=Clean($from)."\n".Clean($subj)."\n".$msg;

  my $charset=Lingua::DetectCharset::Detect($msg);
  Log($msgct,"\n",$charset);

  if($charset ne 'ENG' && $charset ne 'WIN')
  {
    if($charset eq 'KOI8')
    {
      $msg=Convert::Cyrillic::cstocs('koi8','win',$msg);
    }elsif($charset eq 'UTF8')
    {
      $msg=Convert::Cyrillic::cstocs('utf8','win',$msg);
    }
  }

  $msg=~s/(\nMsg=)\x0a/$1/;

  Log("write resp:".length($msg));
  print length($msg)."\n";
  print $msg;
  Log("write resp ok");
}

sub Clean{
  my $str=shift;
  $str=~s/[\x0a\x0d]//g;
  return $str;
};
