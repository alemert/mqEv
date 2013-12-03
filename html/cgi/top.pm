################################################################################
# start html code
################################################################################
sub openHeader
{
  my $dir = $_[0]; shift ;
  my @css = @_ ;

  print "Content-type: text/html; charset=iso-8859-1\n\n";
  print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" 
                              \"http://www.w3.org/TR/html4/loose.dtd\">\n";
  print "<html lang=\"en\">\n";
  print "<head>\n";
  print "<title>mq event</title>\n";
  print "<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">\n";
# print "<style type=\"text/css\">\n";
  foreach my $css (@css)
  {
    print "<link rel=\"stylesheet\" type=\"text/css\" href=\"$dir/$css\" \>\n";
#   print "\@import \"$dir/$css\" screen; \n";
  }
# print "</style>\n" ;
  print "</head>\n";
  print "<body>\n";
}

################################################################################
# end html code
################################################################################
sub closeHeader
{
  print "</div>\n";
  print "</body>\n";
  print "</html>\n";
}

################################################################################
# print top menu
################################################################################
sub showTop
{
  my $active = $_[0]; 
  my $_attr = $_[1];

  print "<div id=main-field>\n";
  print "<div class=top >\n";
  foreach my $attr (sort keys %$_attr )
  {
    my $cmd = $_attr->{$attr}{cmd} ;
    my $dscr = $_attr->{$attr}{dscr} ;
    my $class="top-inactive";
       $class="top-active" if $cmd eq $active ;
    print "<div class=$class>";
    print "<a class=$class href=\"http://krpan/develop/mqev/?cmd=$cmd\"> $dscr " ;
    print "</a></div>\n" ;
  }
  print "</div>\n" ;
}

################################################################################
# end top menu
################################################################################
sub closeTop
{
  print "</div>" ;
}

1;
