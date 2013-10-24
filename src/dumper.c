/******************************************************************************/
/*                     M Q   E V E N T   -   D U M P E R                      */
/*                                                                  */
/*    functions:                                      */
/*      - printAllEventList                        */
/*      - printQmgrEventList                    */
/*      - printEventList                        */
/*      - printMD                            */
/*      - mqbyte2str                    */
/*                                */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdio.h>

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
void printEvent( tMqiItem *_item );
void printMD( PMQMD pmd );
const char* mqbyte2str( PMQBYTE value, MQLONG lng );

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
/*  print qmgr events                                                */
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
/*  print event list                                                */
/******************************************************************************/
void printEventList( tEvent *eventList )
{
  tEvent *event = eventList ;

  while( event )
  {
    printf("    ----------------------------------------\n");
    printEvent( event->item );
    printMD( event->pmd );
    printf("    ----------------------------------------\n");
    event = event->next ;
  }
}

/******************************************************************************/
/*  print Event                                                    */
/******************************************************************************/
void printEvent( tMqiItem *_item )
{
  tMqiItem *item = _item ;

  char* value ;

  while( item )
  {
    switch( item->selector )
    {
      case MQIACF_OPEN_OPTIONS : break;
      case MQIA_APPL_TYPE      : break;
      default :
      {
        printf( "    %-25.25s  ", mqSelector2str( item->selector ) );
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
          printf( "%d", item->value.intVal );
        }
        else
        {
          printf( "%s",  value );
        }
        printf("\n");
      }
    }
    item = item->next ; 
  }
}

/******************************************************************************/
/*  print message description                                    */
/******************************************************************************/
void printMD( PMQMD pmd )
{
  #define printMDln( format, key, value ) \
          {                                 \
	        printf( "    %-20.20s:", key );       \
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
/*  mqbyte to string                              */
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