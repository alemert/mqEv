/******************************************************************************/
/*   M Q   E V E N T   N O D E   U T I L I T I E S                         */
/*                                                                            */
/*   functions:                                                               */
/*    - bag2mqiNode                                                           */
/*    - newMqiItem                                                            */
/*    - addMqiItem                                                            */
/*    - lastMqiItem                                                           */
/*    - setMqiItem                                                            */
/*    - findMqiItem                                                           */
/*    - deleteMqiItem                                                        */
/*    - freeMqiItemValue                                                      */
/*    - addQmgrNode                                                          */
/*    - findQmgrNode                                                        */
/*    - lastQmgrNode                  */
/*    - newEventNode                                */
/*    - addEventNode                              */
/*    - item2event                                            */
/*    - moveMqiItem                                */
/*    - getMsgIdPair                    */
/*                                                              */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdlib.h>

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
#include <msgcat/lgstd.h>
#include <msgcat/lgmqm.h>

#include <mqdump.h>
#include <mqtype.h>
#include <mqbase.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#define _MQEV_NODE_CPP_

#include <node.h>

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define ITEM_STRING_LENGTH      64

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
tMqiItem* newMqiItem( );
tMqiItem* addMqiItem( tMqiItem *anchor );
tMqiItem* lastMqiItem( tMqiItem *anchor );
tMqiItem* setMqiItem( tMqiItem* item, 
                      MQLONG selector, 
                      tMqiItemType type, 
                      tMqiItemVal val );
tMqiItem* findMqiItem( tMqiItem* anchor , MQLONG selector );
void freeMqiItemValue( tMqiItem *item );
void deleteMqiItem( tMqiItem* anchor, tMqiItem* deleteItem );

tQmgrNode* addQmgrNode( char* qmgrName );
tQmgrNode* findQmgrNode( char* qmgrName );
tQmgrNode* lastQmgrNode( );

tEvent* newEventNode();
tEvent* addEventNode( tEvent *anchor, tEvent *node );

void item2event( tQmgrNode *qmgrNode, tMqiItem *anchor, PMQMD pmd );
void moveMqiItem( tMqiItem *item, tMqiItem *anchor, tEvent *event );


/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/
int bag2mqiNode( PMQMD pmd, MQHBAG bag )
{
  MQLONG itemCount;
  MQLONG itemType ;
  MQLONG selector ;

  MQINT32 iVal ;

  MQCHAR itemStrBuff[ITEM_STRING_LENGTH+1];
  MQLONG itemStrLng;
  MQLONG iCCSID;

//MQLONG mqlongVal ;
//char  *mqstrVal  ;

  MQLONG compCode;
  MQLONG reason  ;
  int    i       ;

  tMqiItem *qmgrItem ;
  tMqiItem *anchor   = newMqiItem() ;

  tQmgrNode *qmgrNode = NULL;
  
  // -------------------------------------------------------
  // count items in the bag
  // -------------------------------------------------------
  mqCountItems( bag                ,                     //
                MQSEL_ALL_SELECTORS,                     //
                &itemCount         ,                     //
                &compCode          ,                     //
                &reason           );                     //
                                                         //
  switch( reason )                                       // handle errors in 
  {                                                      //  count items
    case MQRC_NONE : break;                              //
    default :                                            //
    {                                                    //
      logMQCall( DBG, "mqCountItems", reason );          //
      goto _door;                                        //
    }                                                    //
  }                                                      //
  logger( LMQM_ITEM_COUNT, itemCount );                  //
                                                         //
  // -------------------------------------------------------
  // go through all items
  // -------------------------------------------------------
                                                         //
  for( i=0; i<itemCount; i++ )                           //
  {                                                      //
                                                         //
    // -----------------------------------------------------
    // get item type
    // -----------------------------------------------------
    mqInquireItemInfo( bag               ,               // get selector(id)
                       MQSEL_ANY_SELECTOR,               //  and item type
                       i                 ,               //
                       &selector         ,               //
                       &itemType         ,               //
                       &compCode         ,               //
                       &reason          );               //
                                                         //
    switch( reason )                                     //
    {                                                    //
      case MQRC_NONE : break;                            //
      default :                                          //
      {                                                  //
        logMQCall( DBG, "mqCountItems", reason );        //
        goto _door;                                      //
      }                                                  //
    }                                                    //
    logger( LMQM_ITEM_TYPE, mqItemType2str(itemType) );  //
                                                         //
    // -----------------------------------------------------
    // analyse the item type
    // -----------------------------------------------------
    switch( itemType )                                   //
    {                                                    //
      // ---------------------------------------------------
      //  handle INTIGER item
      // ---------------------------------------------------
      case MQITEM_INTEGER :                              //
      {                                                  //
        mqInquireInteger( bag               ,            //
                          MQSEL_ANY_SELECTOR,            //
                          i                 ,            //
                          &iVal             ,            //
                          &compCode         ,            //
                          &reason          );            //
                                                         //
        setMqiItem( addMqiItem( anchor )  ,              //
                      selector            ,              //
                      INTIGER_ITEM        ,              //
                      (tMqiItemVal)(int)iVal );          //
        break;                                           //
      }                                                  //
      // ---------------------------------------------------
      //  handle STRING item
      // ---------------------------------------------------
      case MQITEM_STRING :                               //
      {                                                  //
        mqInquireString( bag               ,             //
                         MQSEL_ANY_SELECTOR,             //
                         i                 ,             //
                         ITEM_STRING_LENGTH,             //
                         itemStrBuff       ,             //
                         &itemStrLng       ,             //
                         &iCCSID           ,             //
                         &compCode         ,             //
                         &reason          );             //
                                                         //
        char* itemStr = (char*) malloc( sizeof(char) *   // alloc string memory
                                        itemStrLng+1 );  //
        memcpy( itemStr, itemStrBuff, itemStrLng );      // copy to str memory
        itemStr[itemStrLng] = '\0';                      //
                                                         //
        setMqiItem( addMqiItem( anchor ),                // add a new item
                      selector          ,                //  with bag value
                      STRING_ITEM       ,                //
                      (tMqiItemVal)(char*) itemStr );    //
        break;                                           //
      }                                                  //
      case MQITEM_BAG :                                  //
      {                                                  //
        break;                                           //
      }                                                  //
      case MQITEM_BYTE_STRING :                          //
      {                                                  //
        break;                                           //
      }                                                  //
      case MQITEM_INTEGER_FILTER :                       //
      {                                                  //
        break;                                           //
      }                                                  //
      case MQITEM_STRING_FILTER :                        //
      {                                                  //
        break;                                           //
      }                                                  //
      case MQITEM_INTEGER64 :                            //
      {                                                  //
        break;                                           //
      }                                                  //
      case MQITEM_BYTE_STRING_FILTER :                   //
      {                                                  //
        break;                                           //
      }                                                  //
    }                                                    //
                                                         //
    switch( reason )                                     //
    {                                                    //
      case MQRC_NONE : break;                            //
      default :                                          //
      {                                                  //
        logMQCall( DBG, "mqInquire???", reason );        //
        goto _door;                                      //
      }                                                  //
    }                                                    //
  }                                                      //
                                                         //
  qmgrItem = findMqiItem( anchor, MQCA_Q_MGR_NAME );     //
  if( qmgrItem == NULL )                                 //
  {                                                      //
    goto _door;                                          //
  }                                                      //
  qmgrNode = addQmgrNode( qmgrItem->value.strVal );      //
  freeMqiItemValue( qmgrItem );            //
  deleteMqiItem( anchor, qmgrItem );                     //
                                                         //
  item2event( qmgrNode, anchor, pmd );                   //
                                                         //
  free(anchor);
  _door:

//  freeItemList( anchor );
  return 0;
}

/******************************************************************************/
/* new item node                                                              */
/******************************************************************************/
tMqiItem* newMqiItem( )
{
  logFuncCall( ) ;

  tMqiItem *item = (tMqiItem*) malloc( sizeof(tMqiItem) );

  if( item == NULL )        //
  {                                          //
    logger( LSTD_MEM_ALLOC_ERROR ) ;         //
    goto _door ;                             //
  }                                          //

  item->selector     = 0 ;
  item->value.strVal =  NULL ;
  item->type         = UNKNOWN_ITEM ;
  item->next         = NULL;

  _door:

  logFuncExit( ) ;
  return item ;
}

/******************************************************************************/
/* add item                                                                   */
/******************************************************************************/
tMqiItem *addMqiItem( tMqiItem *anchor )
{
  logFuncCall( ) ;

  tMqiItem *item = lastMqiItem( anchor );
  if( item == NULL ) goto _door ;

  item->next = newMqiItem() ;
  item = item->next ;
  
  _door:
  return item ;
 
  logFuncExit( ) ;
}

/******************************************************************************/
/* last item                                                                  */
/******************************************************************************/
tMqiItem *lastMqiItem( tMqiItem *anchor )
{
  logFuncCall( ) ;

  tMqiItem *item = anchor ;
 
  if( item == NULL )
  {
    goto _door;
  }

  while( item->next )
  {
    item = item->next ;
  }
 
  _door:

  logFuncExit( ) ;
  return item ;
}

/******************************************************************************/
/* set item                                                                   */
/******************************************************************************/
tMqiItem *setMqiItem( tMqiItem* item, 
                      MQLONG selector, 
                      tMqiItemType type, 
                      tMqiItemVal val )
{
  logFuncCall( ) ;

  if( item == NULL )
  {
    goto _door;
  }

  item->selector = selector ;
  item->value    = val ;
  item->type     = type ;
 
  _door:

  logFuncExit( ) ;
  return item ;
}

/******************************************************************************/
/* set item                                                                   */
/******************************************************************************/
tMqiItem *findMqiItem( tMqiItem* anchor , MQLONG selector )
{
  logFuncCall( ) ;

  tMqiItem *item ;
  tMqiItem *found = NULL;

  if( anchor == NULL )
  {
    goto _door;
  }
#if(0)
  if( anchor->next == NULL )
  {
    goto _door;
  }
#endif

  item = anchor;

  while( item )
  {
    if( (item->selector) == selector )
    {
      found = item;
      break; 
    }
    item = item->next;
  }
 
  _door:

  logFuncExit( ) ;
  return found ;
}

/******************************************************************************/
/* delete item                              */
/******************************************************************************/
void deleteMqiItem( tMqiItem* anchor, tMqiItem* deleteItem )
{
  logFuncCall( ) ;

  tMqiItem *item ;

  if( anchor == NULL )
  {
    goto _door;
  }

  if( anchor->next == NULL )
  {
    goto _door;
  }

  item = anchor;

  while( item )
  {
    if( (item->next) == deleteItem )
    {
      item->next = deleteItem->next; 
      if( item->type == STRING_ITEM )
      {
        free( item->value.strVal );
      }
      free( deleteItem);
      break; 
    }
    item = item->next;
  }
 
  _door:

  logFuncExit( ) ;
  return ;
}

/******************************************************************************/
/* free node value                        */
/******************************************************************************/
void freeMqiItemValue( tMqiItem *item )
{
  if( item->type == STRING_ITEM )
  {
    free( item->value.strVal );
  }
}

/******************************************************************************/
/* add qmgr node                                                        */
/******************************************************************************/
tQmgrNode* addQmgrNode( char* qmgrName )
{
  logFuncCall( ) ;

  tQmgrNode *node;

  if( _gEventList == NULL ) 
  {
    _gEventList = (tQmgrNode*) malloc( sizeof(tQmgrNode) );
    if( _gEventList == NULL )                  //
    {                                          //
      logger( LSTD_MEM_ALLOC_ERROR ) ;         //
      goto _door ;                             //
    }                                          //

    node = _gEventList ;
    sprintf( node->qmgr, qmgrName );
    goto _door;
  }

  node = findQmgrNode( qmgrName );
  if( node ) 
  {
    goto _door;
  }

  node = lastQmgrNode();
  if( node == NULL )
  {
    goto _door;
  }
  node->next = (tQmgrNode*) malloc( sizeof(tQmgrNode) );
  if( node->next == NULL )                   //
  {                                          //
    logger( LSTD_MEM_ALLOC_ERROR ) ;         //
    goto _door ;                             //
  }                                          //
  node = node->next;
  sprintf( node->qmgr, qmgrName );
  node->next = NULL ;
  
  _door: 
  logFuncExit( ) ;
  return node;
}

/******************************************************************************/
/* find qmgr node                                                             */
/******************************************************************************/
tQmgrNode* findQmgrNode( char* qmgrName )
{
  logFuncCall( ) ;

  tQmgrNode *node = NULL;
  tQmgrNode *found = NULL;

  if( _gEventList == NULL ) 
  {
    goto _door;
  }

  node = _gEventList ;

  while( node ) 
  {
    if( strcmp( qmgrName, node->qmgr ) == 0 )
    {
      found = node ;
      break ;
    }
    node = node->next ;
  }

  _door: 
  logFuncExit( ) ;
  return found;
}

/******************************************************************************/
/* last qmgr node                    */
/******************************************************************************/
tQmgrNode* lastQmgrNode( )
{
  logFuncCall( ) ;

  tQmgrNode *last = _gEventList ;

  if( last == NULL )
  {
    goto _door;
  }

  while( last->next )
  {
    last = last->next;
  }

  _door: 
  logFuncExit( ) ;
  return last;
  
}

/******************************************************************************/
/* new event node                                    */
/******************************************************************************/
tEvent* newEventNode()
{
  logFuncCall( ) ;
  tEvent *event; 

  event = (tEvent*) malloc( sizeof(tEvent));
  if( event == NULL )                        //
  {                                          //
    logger( LSTD_MEM_ALLOC_ERROR ) ;         //
    goto _door ;                             //
  }                                          //

  event->item = NULL;
  event->next = NULL;

  _door: 
  logFuncExit( ) ;
  return event;
}

/******************************************************************************/
/* add event node                      */
/******************************************************************************/
tEvent* addEventNode( tEvent *anchor, tEvent *node )
{
  logFuncCall( ) ;

  tEvent *last = anchor ;

  while( last->next )
  {
    last = last->next;
  }

  last->next = node;

  logFuncExit( ) ;
  return last;

}

/******************************************************************************/
/* item to event                  */
/******************************************************************************/
void item2event( tQmgrNode *qmgrNode, tMqiItem *anchor, PMQMD pmd )
{
  logFuncCall( ) ;
  
  tMqiItem *mqiItem;
  tMqiItem *nextMqiItem;
  tMqiItem *tmpItem;

  tEvent   *event  = newEventNode();
  if( event == NULL )                        //
  {                                          //
    goto _door ;                             //
  }                                          //

  
  MQLONG eventList = 0 ;
   
  if( anchor == NULL )
  {
    goto _door;  
  }

  event->pmd = (PMQMD) malloc( sizeof(MQMD) );
  memcpy( event->pmd, pmd, sizeof(MQMD) );
  mqiItem = anchor->next;
   
  while( mqiItem )
  {
    nextMqiItem = mqiItem->next;
    switch ( mqiItem->selector )
    {
      // ---------------------------------------------------
      // items to be ignored
      // ---------------------------------------------------
      case MQIASY_BAG_OPTIONS      : 
      case MQIASY_CODED_CHAR_SET_ID: 
      case MQIASY_TYPE             :
      case MQIASY_VERSION          : 
      case MQIASY_MSG_SEQ_NUMBER   :
      case MQIASY_CONTROL          : 
      case MQIASY_COMP_CODE        :
      {
        freeMqiItemValue( mqiItem );       // free item value for strings
        deleteMqiItem(anchor,mqiItem);     // delete and free item
	break;
      }
      
      // ---------------------------------------------------
      // find out the type of the event (event name), 
      //   to sort the events in one of the event lists
      // ---------------------------------------------------
      case MQIASY_COMMAND :                // if command MQCMD_Q_MGR_EVENT 
      {                                    // MQIASY_REASON has to be evaluated at 
        eventList = mqiItem->value.intVal; // at later stage in order to assign
        freeMqiItemValue( mqiItem );       // stop/start qmgr events to a 
        deleteMqiItem(anchor,mqiItem);     // special list 
        break;                             //
      }                                    //
      // ---------------------------------------------------
      // items to be moved in event list
      // ---------------------------------------------------
      case MQIASY_REASON    :        // reason code 
      case MQIA_APPL_TYPE   :        // application type
      case MQCACF_APPL_NAME :        // application name
      case MQCA_Q_NAME      :        // queue name
      case MQIACF_REASON_QUALIFIER : //
      case MQCACF_USER_IDENTIFIER  : // not authorized (connect)
      case MQIACF_OPEN_OPTIONS     : // not authorized (open)
      case MQCA_BASE_OBJECT_NAME   : // dlq reason
      case MQIA_BASE_TYPE          : // dlq reason (combined with above)
      {
        moveMqiItem( mqiItem, anchor, event );
        break;
      }
      
      default : 
      {
        printf( "missing selector %d\n", (int) mqiItem->selector );
      }
    }
             
    mqiItem = nextMqiItem;
  }
  
  switch( eventList )
  {
    // -----------------------------------------------------
    // queue manager event 
    //   message went to DLQ, put event to single event list
    //   QMGR was stopped / started, put event to QMGR list
    // -----------------------------------------------------
    case MQCMD_Q_MGR_EVENT :                          //
    {                                                 //
      tmpItem=findMqiItem(event->item,MQIASY_REASON); // check the reason for
      switch( tmpItem->value.intVal )                 //   qmgr-event
      {                                               //
        case MQRC_Q_MGR_ACTIVE :                      // if start qmgr
        case MQRC_Q_MGR_NOT_ACTIVE :                  // of stop qmgr
        {                                             // put event to qmgr list
          if( !(qmgrNode->qmgrEvent) )                //
          {                                           //
            qmgrNode->qmgrEvent = event;              //
            break;                                    //
          }                                           //
          addEventNode( qmgrNode->qmgrEvent, event ); //
          break;                                      //
        }                                             //
        default :                                     //
        {                                             // if not start and
          if( !(qmgrNode->singleEvent) )              // not stop qmgr 
          {                                           // (probably DLQ message) 
            qmgrNode->singleEvent = event;            // put event to the 
            break;                                    // single event list
          }                                           //
          addEventNode( qmgrNode->singleEvent, event ); //
          break;                                      //
        }                                             //
      }                                               //
      break;      // switch( eventList )
    }                                                 //
    default :                                         //
    {                                                 //
      printf( "missing selector %d\n", (int) eventList );
    }                                                 //
  }
  
  _door:
  logFuncExit( ) ;
  return;
}


/******************************************************************************/
/* move mqi item                 */
/*    items will be moved from item list to event list       */
/******************************************************************************/
void moveMqiItem( tMqiItem *item, tMqiItem *anchor, tEvent *event )
{
  logFuncCall( ) ;
  tMqiItem *prev = anchor;
  tMqiItem *lastEvItem = NULL ;
  
  while( prev->next != item )
  {
    prev = prev->next;
    if( !prev ) goto _door;
  }

  prev->next = item->next;
  item->next = NULL;
  if( !(event->item) )
  {
    event->item = item;
  }
  else
  {
    lastEvItem = lastMqiItem( event->item );
    lastEvItem->next = item;
  }
  
  _door:
          
  logFuncExit( ) ;
  return;
}

/******************************************************************************/
/*  get message id pairs            */
/******************************************************************************/
PMQBYTE24 getMsgIdPair()
{
  tQmgrNode *qmgrNode = _gEventList ;
  tEvent    *event    ;
 
  while( qmgrNode )
  {
    event = qmgrNode->qmgrEvent ; 
    qmgrNode = qmgrNode->next;
  }
}