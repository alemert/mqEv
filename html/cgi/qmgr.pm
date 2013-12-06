################################################################################
# find all qmgr
################################################################################
sub readEventFiles
{
  my $dir = $_[0] ;
  my %qmgr ;

  foreach $qmgrFile ( <$dir/*.event> )
  {
    my $qmgr = $qmgrFile ;
    $qmgr =~ s/^.+\/(.+)\.(.+)$/$1/ ;
    open QMGR, "$qmgrFile" ;
    <QMGR>;
    foreach my $line (<QMGR>)
    {
      next unless $line =~ /^(0x [0-9A-Fa-f ]+)\s+
                            (\d+)\s+
                            (\w+)\s+
                            (.+)\s*$/x;
      my $msgId  = $1 ;
      my $evTime = $2 ;
      my $reason = $3 ;
      my $dscr   = $4 ;
      $msgId =~ s/\s//g ;

      $qmgr{$qmgr}{$msgId}{time} = $evTime ;
      $qmgr{$qmgr}{$msgId}{reason} = $reason ;
      foreach my $line ( split '\t', $dscr )
      {
        my ($key,$value) = split '=', $line ;
        $qmgr{$qmgr}{$msgId}{detail}{$key} = $value ;
      }
    }
    close QMGR ;
  }
  return \%qmgr ;
}

################################################################################
# print events
################################################################################
sub showEvents
{
  my $_qmgr = $_[0] ;

  print "<div class=qmgr-active>\n";
  foreach my $msgId  ( keys %$_qmgr )
  {
    my $time = $_qmgr->{$msgId}{time};
    $time =~ s/^(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2}).+/$1-$2-$3 $4:$5:$6/;
    print "<div class=event>\n" ;
    print "$time ";
    print "$_qmgr->{$msgId}{reason} \n" ;
    print "<div class=event-detail>\n";
    print "<table>\n";
    foreach my $key ( keys %{$_qmgr->{$msgId}{detail}} )
    {
      print"<tr><td>$key</td><td>$_qmgr->{$msgId}{detail}{$key}</td> </tr>\n";
    }
    print "</table>\n";
    print "</div>\n";
    print "</div>\n" ;
  }
  print "</div>\n";
}


################################################################################
# show all qmgr
################################################################################
sub showQmgr
{
  my $_attr = $_[0] ;
  my $_qmgr = $_[1] ;

  my $activeQmgr = $_attr->{qmgr} if exists $_attr->{qmgr} ;

  print "<div class=qmgr>\n";
  foreach my $qmgr ( sort keys %$_qmgr )
  {
    my $class = "qmgr-inactive" ;
       $class = "qmgr-active" if $qmgr eq $activeQmgr ;
    my $url = setHref $_attr , "qmgr", $qmgr ;
    print "<div class=$class><a class=qmgr href=\"$url\">\n" ;
    print "<img class=qmgr src=\"/develop/icons/red-cross-16.png\">$qmgr\n" ;
    print "</a></div>\n";
    showEvents $_qmgr->{$qmgr} if $qmgr eq $activeQmgr
  }
  print "</div>\n";

}

1;
