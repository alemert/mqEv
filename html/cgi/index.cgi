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

use http;
use top ;
use qmgr;

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

my $_attr = cmdLn ;

my $cmdlnAttr ;
foreach my $attr ( keys %$_attr )
{
  $cmdlnAttr .= "$attr=$_attr->{$attr} <br>\n" ;
}

################################################################################
#   M A I N  
################################################################################

if( exists $_attr->{msgid} )
{
#  $ENV{LD_LIBRARY_PATH}=$ENV{LD_LIBRARY_PATH}.':'."/home/mertale/NetBeansProjects/mqEv/lib/gcc/64/Linux.x86_64/" ;
# system( "/home/mertale/NetBeansProjects/mqEv/bin/gcc/64/Linux.x86_64/mqev --ack $_attr->{msgid} --ini  /home/mertale/NetBeansProjects/mqEv/etc/ini/mqev.apache.ini") ;
  system( "/opt/dbag/mqev --ack $_attr->{msgid} --ini  /home/mertale/NetBeansProjects/mqEv/etc/ini/mqev.apache.ini") ;
  delete $_attr->{msgid} ;
  if( exists $_attr->{qmgr} )
  {
    open ACK, ">$wwwDir/$_attr->{qmgr}.ack" ;
    close ACK ;
  }
}

my $_qmgr = readEventFiles $wwwDir ;

openHeader "/develop/css", "top.css", "qmgr.css", "event.css" ;

showTop $_attr, \%menu;

exists $_attr->{cmd} &&
       $_attr->{cmd} eq 'open' &&
       showQmgr $_attr, $_qmgr ;


closeHeader ;

# print $cmdlnAttr ;