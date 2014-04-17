/******************************************************************************/
/*                     M Q   E V E N T   -   D U M P E R                      */
/*                                                                            */
/*    functions:                                                              */
/*      - printAllEventList                                                   */
/*      - printQmgrEventList                                                  */
/*      - printEventList                                                      */
/*      - printEvent                                                  */
/*      - getEventItem                                            */
/*      - printMD                                                             */
/*      - mqbyte2str                                                          */
/*      - printAllEventTable                                            */
/*      - printQmgrEventTable                                      */
/*      - fPrintEventTopLine                                      */
/*      - printEventTableLine                      */
/*      - touchAckFlag                        */
/*      - flushEventFiles                    */
/*                                            */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>
#undef _USE_XOPEN

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

// ---------------------------------------------------------
// mq
// ---------------------------------------------------------
#include <cmqc.h>
#include <cmqcfc.h>
#include <cmqbc.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include <ctl.h>
#include <lgstd.h>
#include <mqtype.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <dumper.h>
#include <node.h>
#include <mqev.h>

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
#define MQEV_GLOBAL_BUFFER_LNG 1024
char _gBuff[MQEV_GLOBAL_BUFFER_LNG];

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define EVENT_SFX    ".event"
#define TS_SFX       ".ts"
#define ACK_SFX      ".ack"


/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/
#define printEventTopLine( fp, ... )   \
       fPrintEventTopLine( fp ,        \
                         ( sizeof( (char*[]) {NULL,##__VA_ARGS__} ) \
                         / sizeof(char*)-1),                        \
                           ##__VA_ARGS__                            )

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
void printQmgrEventList( tQmgrNode* qmgrNode );
void printEventList( tEvent *eventList );
void printEvent( FILE* fp, tMqiItem *_item );
const char* getEventItem( tMqiItem *_item );
void printMD( PMQMD pmd );
const char* mqbyte2str( PMQBYTE value, MQLONG lng );
int  printQmgrEventTable( const char* dir, tQmgrNode* qmgrEventNode );
void fPrintEventTopLine( FILE *fp, int nr, ... );
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
  logFuncCall( ) ;

  tQmgrNode* qmgrEventNode = _gEventList ;

  while( qmgrEventNode ) 
  {
    printQmgrEventList( qmgrEventNode );
    qmgrEventNode = qmgrEventNode->next;
  }

  logFuncExit( ) ;
}

/******************************************************************************/
/*  print qmgr events                                                         */
/******************************************************************************/
void printQmgrEventList( tQmgrNode* qmgrNode )
{
  logFuncCall( ) ;

  printf("Queue Manager: %s\n", qmgrNode->qmgr );

  printf("  Queue Manager Status\n");
  printEventList( qmgrNode->qmgrEvent );

  printf("  Single Queue Events\n" );
  printEventList( qmgrNode->singleEvent );

  logFuncExit( ) ;
}

/******************************************************************************/
/*  print event list                                                          */
/******************************************************************************/
void printEventList( tEvent *eventList )
{
  logFuncCall( ) ;

  tEvent *event = eventList ;

  while( event )
  {
    printf("    ----------------------------------------\n");
    printEvent( stdout, event->item );
    printMD( event->pmd );
    printf("    ----------------------------------------\n");
    event = event->next ;
  }

  logFuncExit( ) ;
}

/******************************************************************************/
/*  print Event                                                               */
/******************************************************************************/
void printEvent( FILE *fp, tMqiItem *_item )
{
  logFuncCall( ) ;

  tMqiItem *item = _item ;

  char *itemStr;

  while( item )
  {
    itemStr = (char*) getEventItem( item );
    fprintf( fp, "    %-25.25s  %-25.25s", mqSelector2str( item->selector ) ,
                                           itemStr );
    item = item->next ; 
  }
  logFuncExit( ) ;
}

/******************************************************************************/
/*  print event item                                                          */
/******************************************************************************/
const char* getEventItem( tMqiItem *_item )
{
  logFuncCall( ) ;

  tMqiItem *item = _item ;

  char* value ;
  _gBuff[0] = '\0';

  if( item == NULL )
  {
    sprintf( _gBuff, "null" );
    goto _door ;
  }

  switch( item->selector )
  {
    case MQIACF_OPEN_OPTIONS : break;
    case MQIA_APPL_TYPE      : break;
    default :
    {
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
        snprintf( _gBuff, MQEV_GLOBAL_BUFFER_LNG, "%d", item->value.intVal );
      }
      else
      {
        snprintf( _gBuff, MQEV_GLOBAL_BUFFER_LNG,  "%s",  value );
      }
#if(0)
      if( fp == stdout ) { fprintf( fp, "\n" ); }
      else               { fprintf( fp, "\t" ); }
#endif
    }
  }

  _door:

  logFuncExit( ) ;
  return _gBuff ;
}

/******************************************************************************/
/*  print message description                                                 */
/******************************************************************************/
void printMD( PMQMD pmd )
{
  logFuncCall( ) ;

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
  logFuncExit( ) ;
  return ;
}


/******************************************************************************/
/*  mqbyte to string                                                          */
/******************************************************************************/
const char* mqbyte2str( PMQBYTE value, MQLONG lng )
{
  logFuncCall( ) ;

  int i;

  sprintf( _gBuff, "0x" ) ;
  for( i=0; i<lng; i++ )
  {
    sprintf( &(_gBuff[(i+1)*3-1]), " %.2x", value[i] ) ;
  }
  _gBuff[(i+1)*3] = '\0' ;

  logFuncExit( ) ;
  return _gBuff;
}

/******************************************************************************/
/*  print all event in a html table                                           */
/******************************************************************************/
int  printAllEventTable( char* dir )
{
  logFuncCall( ) ;
  int sysRc = 0 ;

  tQmgrNode* qmgrEventNode = _gEventList ;

  while( qmgrEventNode ) 
  {
    sysRc = printQmgrEventTable( dir , qmgrEventNode );
    if( sysRc ) 
    {
      goto _door;
    }
    qmgrEventNode = qmgrEventNode->next;
  }

  _door:

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*  print queue manager event table                                         */
/******************************************************************************/
int printQmgrEventTable( const char *dir, tQmgrNode* qmgrNode )
{
  logFuncCall( ) ;

  int sysRc = 0;
  FILE *fp ;

  char fileName[NAME_MAX];      // ignore the length of the path 
  char *p;
                                                   //  
  snprintf( fileName, NAME_MAX-strlen(EVENT_SFX),  // build a name of the file
	        "%s/%s ", dir, qmgrNode->qmgr  );  // {htmldir}/{qmgr}.event
  p = strchr( fileName, ' ' );                     // find 1st blank
  if( p ) sprintf( p, EVENT_SFX ) ;                // add suffix
                                                   //
  fp = fopen( fileName, "w" );                     //open the file 
  if( !fp )                                        // handle error
  {                                                //
    logger( LSTD_OPEN_FILE_FAILED, fileName );     //
    sysRc = 1;                                     //
    goto _door;                                    //
  }                                                //
                                                   //
  printEventTopLine( fp, "Message Id"   ,          //
                         "Date/Time"    ,          //
                         "MQIASY_REASON",          //
                         "Description" );          //
                                                   //
  printEventTableLine( fp, qmgrNode->singleEvent );//
  printEventTableLine( fp, qmgrNode->qmgrEvent );  //
  printEventTableLine( fp, qmgrNode->doubleEvent );//
                                                   //
  fprintf( fp, "\n" );                             //
  fclose( fp );                                    //
                                                   //
  _door:

  logFuncExit( ) ;
  return sysRc ;
//printTableTopLine(fd,)
}

/******************************************************************************/
/*  print evnt table top line                                            */
/******************************************************************************/
void fPrintEventTopLine( FILE *fp, int nr, ... )
{
  logFuncCall( ) ;

  va_list argp ;
  int i;

  char *text ;

  va_start( argp, nr );

  for( i=0; i<nr; i++ )
  {
    text = va_arg( argp, char* );
    fprintf( fp, "%s\t", text );
  }

  va_end( argp );
  fprintf( fp, "\n" );

  logFuncExit( ) ;
}

/******************************************************************************/
/*  print queue manager event table line                                      */
/******************************************************************************/
void printEventTableLine( FILE* fp, tEvent* eventList )
{
  logFuncCall( ) ;

  tEvent *event = eventList ;
  tMqiItem *item;
  char *itemStr;

 struct tm  gmtTs;
 struct tm  locTs;
 time_t     gmtTime;
 time_t     locTime;
 char       timeStr[64];
 char       locStr[64];

 time_t currTime = time(0);
 time_t utcOffset = currTime - mktime(gmtime(&currTime));

  while( event )
  {
    sprintf( timeStr, "%8.8s%6.6s", event->pmd->PutDate, 
	                            event->pmd->PutTime);
    strptime( timeStr, "%Y%m%d%H%M%S", &gmtTs);
    gmtTime = mktime( &gmtTs );
    locTime = gmtTime + utcOffset;
    locTs = *localtime( &locTime );
    strftime( locStr, 64, "%Y%m%d%H%M%S",&locTs);

    // -----------------------------------------------------
    // print message descriptor data
    // -----------------------------------------------------
    fprintf( fp, "%s\t",mqbyte2str(event->pmd->MsgId,24));

    fprintf( fp, "%8.8s%6.6s\t", event->pmd->PutDate, 
	                         event->pmd->PutTime );

    fprintf( fp, "%s\t", locStr );

    item = findMqiItem( event->item, MQIASY_REASON ) ;
    itemStr = (char*) getEventItem( item );
    fprintf( fp, "%s", itemStr );

    item = event->item ;
    while( item )
    {
#if(0)
      if( item->selector == MQIASY_REASON )
      {
        item = item->next ;
        continue ;
      }
#endif
      itemStr = (char*) getEventItem( item );
      if( *itemStr != '\0')
      {
        fprintf( fp, "\t%s=%s", mqSelector2str( item->selector ), itemStr );
      }
      item = item->next ;
    }
    fprintf( fp, "\n" );

    event = event->next;
  }

  logFuncExit( ) ;
}


/******************************************************************************/
/*  touch event flag file                                     */
/******************************************************************************/
void touchEventFlag(char* dir, char movedMsgQmgr[][MQ_Q_MGR_NAME_LENGTH+1])
{
  FILE *fp;

  char fileName[NAME_MAX] ;
  int i;

  if( dir == NULL )
  {
    goto _door ;
  }

  for( i=0; i<TRANSACTION_SIZE; i++ )
  {
    if( movedMsgQmgr[i][0] == '\0' ) break ;
    sprintf( fileName, "%s/%s"EVENT_SFX""TS_SFX, dir, movedMsgQmgr[i] );

    fp = fopen( fileName, "w" );                     //open the file 
    if( !fp )                                        // handle error
    {                                                //
      logger( LSTD_OPEN_FILE_FAILED, fileName );     //
      goto _door;                                    //
    }                                                //
    fclose( fp );
  }

  _door :
  
  return ;
}

/******************************************************************************/
/*  touch acknowledge flag file                        */
/******************************************************************************/
void touchAckFlag( char* dir, char *qmgrName )
{
  FILE *fp;

  char fileName[NAME_MAX] ;

  if( dir == NULL )
  {
    goto _door ;
  }

  if( qmgrName == NULL )
  {
    goto _door ;
  }

  sprintf( fileName, "%s/%s"ACK_SFX""TS_SFX, dir, qmgrName ) ;

  fp = fopen( fileName, "w" );                     //open the file 
  if( !fp )                                        // handle error
  {                                                //
    logger( LSTD_OPEN_FILE_FAILED, fileName );     //
    goto _door;                                    //
  }                                                //
  fclose( fp );

  _door :
  
  return ;
}

/******************************************************************************/
/*   flush old events from existing files                  */
/*                        */
/*   description:           */
/*     flash all files on $wwwDir (i.g. /var/mq_misc/www)  */
/******************************************************************************/
int flushEventFiles( char* wwwDir )
{
  int sysRc = 0;

  DIR *dp ;  
  FILE *fp;
  struct dirent *dirEntry ;

  char *sfx ;
  char fileName[NAME_MAX] ;

  dp = opendir( wwwDir );
  if( !dp )
  {
    logger( LSTD_OPEN_FILE_FAILED, wwwDir );     //
    sysRc = 1;
    goto _door ;
  }

  while( 1 ) 
  {
    dirEntry = readdir( dp );
    if( !dirEntry ) break ; 

    sfx = strstr( dirEntry->d_name, EVENT_SFX ) ;
    if( !sfx ) continue ;

    if( strlen(sfx) != strlen(EVENT_SFX) ) continue ;

    sprintf( fileName, "%s/%s", wwwDir, dirEntry->d_name );

    fp = fopen( fileName, "w" );
    if( !fp )
    {
      logger( LSTD_OPEN_FILE_FAILED, fileName );     //
      sysRc = 1;
      goto _door ;
    }

    fclose(fp) ;
  }

  _door:

  closedir( dp );

  return sysRc ;
}

