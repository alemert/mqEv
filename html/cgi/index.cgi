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
#   M A I N  
################################################################################

openHeader "/develop/css", "top.css", "qmgr.css";
#foreach my $key (keys %attr )
#{
#  print "$key $attr{$key}<br>\n";
#}
my $active = "" ;
$active = $attr{cmd} if( exists $attr{cmd} ) ;
showTop $active, \%menu;

my $_qmgr = readEventFiles $wwwDir ;
showQmgr $_qmgr ;

closeHeader ;