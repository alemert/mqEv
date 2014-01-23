################################################################################
# find all qmgr
################################################################################
sub readEventFiles
{
  my $dir = $_[0] ;
  my %qmgr ;

  foreach $qmgrFile ( <$dir/*.event> )
  {
    my $qmgr = $qmgrFile ;              # get qmgr from file name
    $qmgr =~ s/^.+\/(.+)\.(.+)$/$1/ ;   #
    $qmgr{$qmgr}{foo} = '' ;            # create empty hash node
    delete $qmgr{$qmgr}{foo} ;          #  create empty hash node
                                        #
    my @stat = stat $qmgrFile.".ts" ;   # calculate the age of the event file 
    my @fileAge = localtime( $stat[9] );#
    my $ss   = $fileAge[0] ;            #
    my $mm   = $fileAge[1] ;            #
    my $hh   = $fileAge[2] ;            #
    my $dd   = $fileAge[3] ;            #
    my $MM   = $fileAge[4] ;            #
    my $YYYY = $fileAge[5] + 1900 ;     #
    $qmgr{$qmgr}{EVENT_TS} = sprintf( "%4d-%2d-%2d!%2d:%2d:%2s" , 
                                 $YYYY,$MM,$dd,$hh,$mm,$ss); 
    $qmgr{$qmgr}{EVENT_TS} =~ s/ /0/g;  #
    $qmgr{$qmgr}{EVENT_TS} =~ s/!/ /g;  #
                                        #
    my $ackFile = $qmgrFile ;
    $ackFile =~ s/\.event$/.ack/;

    my @stat = stat $ackFile ;          # calculate the age of the 
    my @fileAge = localtime( $stat[9] );#  acknowledge file 
    my $ss   = $fileAge[0] ;            #
    my $mm   = $fileAge[1] ;            #
    my $hh   = $fileAge[2] ;            #
    my $dd   = $fileAge[3] ;            #
    my $MM   = $fileAge[4] ;            #
    my $YYYY = $fileAge[5] + 1900 ;     #
    $qmgr{$qmgr}{ACK_TS} = sprintf( "%4d-%2d-%2d!%2d:%2d:%2s" , 
                                 $YYYY,$MM,$dd,$hh,$mm,$ss); 
    $qmgr{$qmgr}{ACK_TS} =~ s/ /0/g;  #
    $qmgr{$qmgr}{ACK_TS} =~ s/!/ /g;  #
                                        #
    open QMGR, "$qmgrFile" ;
    <QMGR>;
    foreach my $line (<QMGR>)
    {
      next unless $line =~ /^(0x [0-9A-Fa-f ]+)\s+
                            (\d+)\s+
                            (\d+)\s+
                            (\w+)\s+
                            (.+)\s*$/x;
      my $msgId  = $1 ;
      my $gmtTime = $2 ;
      my $locTime = $3 ;
      my $reason = $4 ;
      my $dscr   = $5 ;
      $msgId =~ s/\s//g ;

      $qmgr{$qmgr}{$msgId}{time} = $locTime ;
      $qmgr{$qmgr}{$msgId}{reason} = $reason ;
      foreach my $line ( split '\t', $dscr )
      {
        my ($key,$value) = split '=', $line ;
        $value =~ s/\s*$//g;
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

  my @msgId = keys %$_qmgr ;

  print "<div class=event-all>\n";
  if( scalar @msgId == 0 )
  {
    print "<div class=event-empty><p>no open events found</p></div>\n";
  }
  foreach my $msgId  ( @msgId )
  {
    my $time = $_qmgr->{$msgId}{time};
    $time =~ s/^(\d{4})(\d{2})(\d{2})
                (\d{2})(\d{2})(\d{2}).*$/$1-$2-$3 $4:$5:$6/x;
    print "<div class=event>\n" ;

    print "<div class=event-reason>";
    print "<table class=event-reason><tr>\n";
    print "<td class=event-time>$time</td>\n";
    print "<td class=event-reason>\n";
    print "<form action=\"\" method=POST>\n";
    print "<button class=event-reason type=submit name=msgid value=$msgId> ";
    print "ACKNOWLEDGE<br>$_qmgr->{$msgId}{reason}\n";
    print "</button>\n";
    print "</form>\n";
    print "</td>\n" ;
    print "</tr></table>\n";
    print "</div>\n";

    my $rKey = "MQIASY_REASON";
    print "<div class=event-detail>\n";
    print "<table rules=\"rows\" class=event-detail>\n";
    print "<tr class=event-detail>\n";
    print "<td class=event-reason>$rKey</td>";
    print "<td class=event-detail>$_qmgr->{$msgId}{detail}{$rKey}</td> </tr>\n";
    delete $_qmgr->{$msgId}{detail}{$rKey} ;

    foreach my $key ( keys %{$_qmgr->{$msgId}{detail}} )
    {
      print "<tr class=event-detail>\n";
      print "<td class=event-reason>$key</td>";
      print "<td class=event-detail>$_qmgr->{$msgId}{detail}{$key}</td> </tr>\n";
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

  print "<div class=qmgr-all>\n";
  foreach my $qmgr ( sort keys %$_qmgr )
  {
    my $img = "/develop/icons/red-cross-16.png" ;
       $img = "/develop/icons/tick-icon-16.png" if scalar keys %{$_qmgr->{$qmgr}} < 3 ;

    my $class = "qmgr-inactive" ;
       $class = "qmgr-active" if $qmgr eq $activeQmgr ;

    my $url = setHref $_attr , "qmgr", $qmgr ;

    my $evntCnt = ( scalar keys %{$_qmgr->{$qmgr}} ) -2 ;
    if( $evntCnt == 0 )    { $evntCnt = "no event" ; }
    elsif( $evntCnt == 1 ) { $evntCnt = "1 event"  ; }
    else { $evntCnt .= " events" ; }
    $evntCnt = $evntCnt." since" ;

    print "<div class=$class>\n";
    print "<div class=qmgr>";
    print "<a class=qmgr href=\"$url\"><img class=qmgr src=\"$img\">$qmgr </a>";
    print "</div>\n";
#   print "<div class=qmgr-age>$evntCnt<br>$_qmgr->{$qmgr}{EVENT_TS}</div>\n";
#   print "<div class=qmgr-age>" ;
    print "<table class=qmgr-age>";
    print "<tr><td>$evntCnt</td><td>last acknowledge</td></tr>";
    print "<tr><td>$_qmgr->{$qmgr}{EVENT_TS}</td>";
    print "<td>$_qmgr->{$qmgr}{ACK_TS}</td></tr>";
    print "</table>";
#   print "</div>\n";
    print "</div>\n";
    delete $_qmgr->{$qmgr}{EVENT_TS};
    delete $_qmgr->{$qmgr}{ACK_TS};
    showEvents $_qmgr->{$qmgr} if $qmgr eq $activeQmgr;
  }
  print "</div>\n";

}

1;
