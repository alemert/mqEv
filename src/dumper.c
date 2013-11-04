/******************************************************************************/
/*                     M Q   E V E N T   -   D U M P E R                      */
/*                                                                            */
/*    functions:                                                              */
/*      - printAllEventList                                                  */
/*      - printQmgrEventList                                            */
/*      - printEventList                                                */
/*      - printEvent                              */
/*      - printEventItem                        */
/*      - printMD                                                */
/*      - mqbyte2str                                              */
/*      - printAllEventTable                                */
/*      - printQmgrEventTable                          */
/*      - printEventTopLine                    */
/*      - printEventTableLine            */
/*                                      */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>

// ---------------------------------------------------------
// mq
// ---------------------------------------------------------
#include <cmqc.h>
#include <cmqcfc.h>
#include <cmqbc.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include <mqtype.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <node.h>

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
char _gBuff[128];

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
void printQmgrEventList( tQmgrNode* qmgrNode );
void printEventList( tEvent *eventList );
void printEvent( FILE* fp, tMqiItem *_item );
void printEventItem( FILE *fp, tMqiItem *_item );
void printMD( PMQMD pmd );
const char* mqbyte2str( PMQBYTE value, MQLONG lng );
void printQmgrEventTable( FILE* fp, tQmgrNode* qmgrEventNode );
void printEventTopLine( FILE *fp, int nr, ... );
void printEventTableLine( FILE* fp, tEvent* eventList );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* print all events                                                           */
/******************************************************************************/
void printAllEventList()
{
  tQmgrNode* qmgrEventNode = _gEventList ;

  while( qmgrEventNode ) 
  {
    printQmgrEventList( qmgrEventNode );
    qmgrEventNode = qmgrEventNode->next;
  }
}

/******************************************************************************/
/*  print qmgr events                                                         */
/******************************************************************************/
void printQmgrEventList( tQmgrNode* qmgrNode )
{
  printf("Queue Manager: %s\n", qmgrNode->qmgr );

  printf("  Queue Manager Status\n");
  printEventList( qmgrNode->qmgrEvent );

  printf("  Single Queue Events\n" );
  printEventList( qmgrNode->singleEvent );
}

/******************************************************************************/
/*  print event list                                                          */
/******************************************************************************/
void printEventList( tEvent *eventList )
{
  tEvent *event = eventList ;

  while( event )
  {
    printf("    ----------------------------------------\n");
    printEvent( stdout, event->item );
    printMD( event->pmd );
    printf("    ----------------------------------------\n");
    event = event->next ;
  }
}

/******************************************************************************/
/*  print Event                                                               */
/******************************************************************************/
void printEvent( FILE *fp, tMqiItem *_item )
{
  tMqiItem *item = _item ;

  while( item )
  {
    fprintf( fp, "    %-25.25s  ", mqSelector2str( item->selector ) );
    printEventItem( fp, item );
    item = item->next ; 
  }
}

/******************************************************************************/
/*  print Event                                                               */
/******************************************************************************/
void printEventItem( FILE *fp, tMqiItem *_item )
{
  tMqiItem *item = _item ;

  char* value ;

  if( item == NULL )
  {
    fprintf( fp, "null" );
    goto _door ;
  }

  switch( item->selector )
  {
    case MQIACF_OPEN_OPTIONS : break;
    case MQIA_APPL_TYPE      : break;
    default :
    {
 //   fprintf( fp, "    %-25.25s  ", mqSelector2str( item->selector ) );
      if( item->type == INTIGER_ITEM )
      {
        value = (char*) itemValue2str( item->selector, item->value.intVal );
      }
      else
      {
        value = item->value.strVal ;
      }
  
      if( value == NULL )
      {
        fprintf( fp, "%d", item->value.intVal );
      }
      else
      {
        fprintf( fp, "%s",  value );
      }
      fprintf( fp, "\n" );
    }
  }

  _door:
  return;
}

/******************************************************************************/
/*  print message description                                              */
/******************************************************************************/
void printMD( PMQMD pmd )
{
  #define printMDln( format, key, value )       \
          {                                     \
	        printf( "    %-20.20s:", key ); \
	        printf( format, value );        \
	        printf( "\n" );                 \
	      }

//printMDln("%-4.4s"  ,"Structure identifier",pmd->StrucId       );           
//printMDln("%-25.25s","Structure version"   ,(char*)mqmdVer2str(pmd->Version));
//printMDln("%-25.25s","Report options",(char*)mqReportOption2str(pmd->Report));
//printMDln("%-25.25s","Message type"  ,(char*)mqMsgType2str(pmd->MsgType));           
//printMDln("%9.9d"   ,"Expiry"        ,pmd->Expiry);
//printMDln("%-25.25s","Feedback code" ,(char*)mqFeedback2str( pmd->Feedback));
//printMDln("%-25.25s","Encoding"      ,(char*)mqEncondig2str(pmd->Encoding));
//printMDln("%-25.25s","CCSID"        ,(char*)mqCCSID2str(pmd->CodedCharSetId));
//printMDln("%-8.8s"  ,"Message Format",pmd->Format        );            
//printMDln("%-25.25s","Priority"  ,(char*)mqPriority2str(pmd->Priority));          
//printMDln("%-25.25s","Persist"   ,(char*)mqPersistence2str(pmd->Persistence));
  printMDln("%s"      ,"Message id",mqbyte2str(pmd->MsgId,24));
//printMDln("%s"      ,"Correlation id" ,mqbyte2str(pmd->CorrelId,24));
//printMDln("%9.9d"   ,"Backout counter",pmd->BackoutCount  );
//printMDln("%-48.48s","Reply queue"    ,pmd->ReplyToQ      );
//printMDln("%-48.48s","Reply qmgr"     ,pmd->ReplyToQMgr   );
//printMDln("%-12.12s","User identifier",pmd->UserIdentifier);
//printMDln("%s"      ,"Account token"  ,mqbyte2str(pmd->AccountingToken,32));
//printMDln("%-32.32s","Application identity",pmd->ApplIdentityData );
//printMDln("%-25.25s","Put Type"  ,(char*)mqPutApplType2str(pmd->PutApplType));
//printMDln("%-28.28s","Application name"    ,pmd->PutApplName   );       
  printMDln("%-8.8s"  ,"Put Date / Time"     ,pmd->PutDate       );
  printMDln("%-8.8s"  ,"Put time"            ,pmd->PutTime       );
//printMDln("%-4.4s"  ,"Application origin"  ,pmd->ApplOriginData);

  // -------------------------------------------------------
  // msg dscr version 2 or higher
  // -------------------------------------------------------
  if( pmd->Version < MQMD_VERSION_2 ) goto _door ;

//printMDln("%s"      ,"Group identifier"   ,mqbyte2str( pmd->GroupId, 24 ));
//printMDln("%9.9d"   ,"Logical SeqNr group",pmd->MsgSeqNumber  );
//printMDln("%9.9d"   ,"PhysMsg Offset"     ,pmd->Offset        );
//printMDln("%-25.25s","Message flags"  ,(char*)mqMsgFlag2str(pmd->MsgFlags));
//printMDln("%9.9d"   ,"Orig Message Length",pmd->OriginalLength);

  _door:
	return ;
}


/******************************************************************************/
/*  mqbyte to string                                        */
/******************************************************************************/
const char* mqbyte2str( PMQBYTE value, MQLONG lng )
{
  int i;

  sprintf( _gBuff, "0x" ) ;
  for( i=0; i<lng; i++ )
  {
    sprintf( &(_gBuff[(i+1)*3-1]), " %.2x", value[i] ) ;
  }
  _gBuff[(i+1)*3] = '\0' ;

  return _gBuff;
}

/******************************************************************************/
/*  print all event in a html table                    */
/******************************************************************************/
void  printAllEventTable()
{
  FILE* fp=stdout;
//fprintf( fp, "Contet-type: text/html \n\n" );
  fprintf( fp, "<html>\n");
  fprintf( fp, "<title>MQ Events</title>\n");

  tQmgrNode* qmgrEventNode = _gEventList ;

  while( qmgrEventNode ) 
  {
    printQmgrEventTable( fp, qmgrEventNode );
    qmgrEventNode = qmgrEventNode->next;
  }
  fprintf( fp, "</html>\n");
}

/******************************************************************************/
/*  print queue manager event table                            */
/******************************************************************************/
void printQmgrEventTable( FILE* fp, tQmgrNode* qmgrNode )
{
  fprintf( fp, "<div align=\"center\">\n" );
  fprintf( fp, "<h1>%s</h1>\n", qmgrNode->qmgr );
  fprintf( fp, "</div>\n" );

  fprintf( fp, "<table border=\"1\">\n" );

  printEventTopLine( fp, 1,"Date/Time", "MQIASY_REASON", "Message Id" );
//printEventList( qmgrNode->qmgrEvent );
  printEventTableLine( fp, qmgrNode->singleEvent );

  fprintf( fp, "</table>\n" );
//printTableTopLine(fd,)
}

/******************************************************************************/
/*  print evnt table top line      */
/******************************************************************************/
void printEventTopLine( FILE *fp, int nr, ... )
{
  va_list argp ;
  int i;

  int id ;

  //check for example in initypes.h

  fprintf( fp, "<tr>\n" );
//fprintf( fp, "<th>date / time</th>");
  va_start( argp, nr );

  for( i=0; i<nr; i++ )
  {
    id = va_arg( argp, char* );
    fprintf( fp, "<th> %s </th>" );
  }

  va_end( argp );
//fprintf( fp, "<th>message  id</th>");
  fprintf( fp, "\n</tr>\n" );
}

/******************************************************************************/
/*  print queue manager event table line                         */
/******************************************************************************/
void printEventTableLine( FILE* fp, tEvent* eventList )
{
  tEvent *event = eventList ;
  tMqiItem *item;

  while( event )
  {
    fprintf( fp, "<tr>" );
    // -----------------------------------------------------
    // print message descriptor data
    // -----------------------------------------------------
    fprintf( fp, "<td>%8.8s %6.6s</td>", event->pmd->PutDate, 
	                                event->pmd->PutTime );
    
    item = findMqiItem( event->item, MQIASY_REASON ) ;
    fprintf( fp, "<td>");
      printEventItem( fp, item );
      deleteMqiItem( item );
    fprintf( fp, "</td>");

//  printMD( event->pmd );
//  printEvent( event->item );
    fprintf( fp, "<td>%s</td>",mqbyte2str(event->pmd->MsgId,24));
    fprintf( fp, "</tr>\n" );
    event = event->next;
  }
}