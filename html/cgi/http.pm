################################################################################
# general functions for http
################################################################################

################################################################################
#   L I B R A R I E S  
################################################################################
use strict;

################################################################################
#   G L O B A L S  
################################################################################

################################################################################
#   C O M M A N D   L I N E  
################################################################################

sub cmdLn
{
  my $urlAttr =  $ENV{'QUERY_STRING'};

  $ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;
  my $buffer ;
  if( $ENV{'REQUEST_METHOD'} eq 'POST' )
  {
    read( STDIN, $buffer, $ENV{CONTENT_LENGTH});
  }

  my %attr;
  my @attrPairs = split /&/, $urlAttr ;
  push @attrPairs, split /&/, $buffer ;
  foreach my $pair (@attrPairs)
  {
    chomp $pair ;
    $pair =~ /^(.+)=(.+)$/;
    my $key = $1 ;
    my $val = $2 ;
    $attr{$key} = $val ;
  } 


  return \%attr ;
}

################################################################################
#
################################################################################
sub setHref
{
  my $_attr = $_[0] ;
  my $key = $_[1] ;
  my $value = $_[2] ;

  my %url = %$_attr ;

  my $href = "?" ;

  if( exists $url{$key} &&
             $url{$key} eq $value )
  {
    delete $url{$key};
  }
  else
  {
    $url{$key} = $value ;
  }

  foreach my $attr ( keys %url )
  {
    $href .= $attr.'='.$url{$attr}.'&amp;' ;
  }

  $href =~ s/\&amp;$// ;
  $href = "" if $href eq "?" ;

  return $href ;
}

1;