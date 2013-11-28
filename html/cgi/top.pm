
sub openHeader
{
  my $dir = $_[0]; shift ;
  my @css = @_ ;

  print "Content-type: text/html; charset=iso-8859-1\n\n";
  print "<html>\n";
  print "<head>\n";
  print "<style type=\"text/css\">\n";
  foreach my $css (@css)
  {
    print "\@import \"$dir/$css\" screen; \n";
  }
  print "</style>\n" ;
  print "</head>\n";
  print "<body>\n";
}

sub closeHeader
{
  print "</body>\n";
  print "</html>\n";
}

sub openTop
{
  my @attr = @_ ;

  print "<div id=top >\n" ;
  foreach my $attr (@attr)
  {
    print " $attr";
  }
}

sub closeTop
{
  print "</div>" ;
}

1;