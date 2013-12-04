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
        $qmgr{$qmgr}{$msgId}{decr}{$key} = $value ;
      }
    }
    close QMGR ;
  }
  return \%qmgr ;
}
################################################################################
# show all qmgr
################################################################################
sub showQmgr
{
  my $_attr = $_[0] ;
  my $_qmgr = $_[1] ;

  setHref $_attr ;

  print "<div class=qmgr>\n";
  foreach my $qmgr ( sort keys %$_qmgr )
  {
    print "<div class=qmgr-inactive><a >\n" ;
    print "<img class=qmgr src=\"/develop/icons/red-cross-16.png\">$qmgr\n" ;
    print "</a></div>\n";
  }
  print "</div>\n";

}

1;
