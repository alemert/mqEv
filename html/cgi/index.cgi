#!/usr/bin/perl
################################################################################
# mq event web interface
################################################################################

################################################################################
#   L I B R A R I E S  
################################################################################
use strict;

use CGI qw(:standard);
use CGI::Carp qw(warningsToBrowser fatalsToBrowser);

use top ;
use qmgr ;

################################################################################
#   G L O B A L S  
################################################################################
my %menu = 
(
 0=> { "cmd"  =>"open",
       "dscr" => "open events"},
 1=> { "cmd"  => "history" ,
       "dscr" => "event history", },
 2=> { "cmd"  => "report",
       "dscr" => "event reports" }
);

my $wwwDir = "/var/mq_misc/www/" ;

################################################################################
#   C O M M A N D   L I N E  
################################################################################

my $urlAttr =  $ENV{'QUERY_STRING'};

$ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;

my %attr;
my @attrPairs = split /&/, $urlAttr ;
foreach my $pair (@attrPairs)
{
  chomp $pair ;
  $pair =~ /^(.+)=(.+)$/;
  my $key = $1 ;
  my $val = $2 ;
  $attr{$key} = $val ;
} 

################################################################################
#
################################################################################
sub setHref
{
  my $_attr = $_[0] ;
  my $key = $_[1] ;
  my $value = $_[2] ;

  my $href = "?" ;

  if( exists $_attr->{$key} &&
             $_attr->{key} eq $value )
  {
    delete $_attr->{key};
  }
  else
  {
    $_attr->{key} = $value ;
  }

  foreach my $attr ( keys %$_attr )
  {
    $href .= $key.'='.$value.'&' ;
  }

  $href =~ s/&$// ;
  $href = "" if $href eq "?" ;

  return $href ;
}

################################################################################
#   M A I N  
################################################################################

my $_qmgr = readEventFiles $wwwDir ;

openHeader "/develop/css", "top.css", "qmgr.css";
showTop \%attr, \%menu;

exists %attr->{cmd} &&
       %attr->{cmd} eq 'open' &&
       showQmgr \%attr, $_qmgr ;


closeHeader ;