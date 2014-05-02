/******************************************************************************/
/*   M Q   E V E N T   N O D E   U T I L I T I E S                            */
/*                      */
/*   application: mqevent                                                     */
/*   module     : node                                           */
/*                                                                            */
/*   functions:                                                               */
/*    - bag2mqiNode                                                           */
/*    - newMqiItem                                                            */
/*    - addMqiItem                                                            */
/*    - lastMqiItem                                                           */
/*    - setMqiItem                                                            */
/*    - findMqiItem                                                           */
/*    - deleteMqiItem                                                         */
/*    - freeMqiItemValue                                                      */
/*    - addQmgrNode                                                           */
/*    - findQmgrNode                                                          */
/*    - lastQmgrNode                                                          */
/*    - newEventNode                                                          */
/*    - addEventNode                                                          */
/*    - item2event                                                            */
/*    - moveMqiItem                                                           */
/*    - getMsgIdPair                                                          */
/*    - matchEventReason                                                      */
/*    - freeEventTree                                      */
/*    - freeItemTree                                  */
/*    - freeItemList                                    */
/*    - findEmtpyQueueManager                        */
/*                                                                            */
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
#include <msgcat/lgmqe.h>

#include <mqdump.h>
#include <mqtype.h>
#include <mqbase.h>

// ---------------------------------------------------------
// own
// ---------------------------------------------------------
#include <mqtype.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#define _MQEV_NODE_C_MODULE_

//#include <level.h>
#include <node.h>

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define ITEM_STRING_LENGTH      64
#define MSGID_PAIR_AMOUNT      128

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
MQBYTE24 _gMsgIdPair[MSGID_PAIR_AMOUNT+1];

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
//tMqiItem* findMqiItem( tMqiItem* anchor , MQLONG selector );
void freeMqiItemValue( tMqiItem *item );
//void deleteMqiItem( tMqiItem* anchor, tMqiItem* deleteItem );

tQmgrNode* addQmgrNode( char* qmgrName );
tQmgrNode* findQmgrNode( char* qmgrName );
tQmgrNode* lastQmgrNode( );

tEvent* newEventNode();
tEvent* addEventNode( tEvent *anchor, tEvent *node );

int item2event( tQmgrNode *qmgrNode, tMqiItem *anchor, PMQMD pmd );
void moveMqiItem( tMqiItem *item, tMqiItem *anchor, tEvent *event );

MQLONG matchEventReason( MQLONG reason );

void freeItemTree( tEvent *_eventNode );
void freeItemList( tMqiItem *_itemNode );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*   mqi bag to mqi node                                                      */
/*                                                                            */
/*   description:                                                             */
/*     - analyze mq bag respecting Item Type                                  */
/*     - add item to the event list of items                                  */
/*     - add event list of items to the queue manager event list              */
/*                                                                            */
/*   return code                                                              */
/*        0: OK                                                               */
/*       -3: unknown item type                                                */
/*     forward RC from item2event                                             */
/*        0: OK                                                               */
/*        1: empty input (MQI-Item empty                                      */
/*       -1: missing code, message should go to the error queue               */
/*       -2: missing code, message should go to the error queue               */
/*                                                                            */
/******************************************************************************/
int bag2mqiNode( PMQMD pmd, MQHBAG bag )
{
  int sysRc = 0 ;

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
  // copy item to the event list of items
  // -------------------------------------------------------
                                                         //
  for( i=0; i<itemCount; i++ )                           //
  {                                                      //
                                                         //
    // -----------------------------------------------------
    // get item type
    // -----------------------------------------------------
    mqInquireItemInfo( bag               ,             // get selector(id)
                       MQSEL_ANY_SELECTOR,             //  and item type
                       i                 ,             //
                       &selector         ,             //
                       &itemType         ,             //
                       &compCode         ,             //
                       &reason          );             //
                                                       //
    switch( reason )                                   //
    {                                                  //
      case MQRC_NONE : break;                          //
      default :                                        //
      {                                                //
        logMQCall( DBG, "mqCountItems", reason );      //
        goto _door;                                    //
      }                                                //
    }                                                  //
    logger( LMQM_ITEM_TYPE, mqItemType2str(itemType)); //
                                                       //
    // -----------------------------------------------------
    // analyse the item type
    // -----------------------------------------------------
    switch( itemType )                                 //
    {                                                  //
      // ---------------------------------------------------
      //  handle INTIGER item
      // ---------------------------------------------------
      case MQITEM_INTEGER :                            //
      {                                                //
        mqInquireInteger( bag               ,          //
                          MQSEL_ANY_SELECTOR,          //
                          i                 ,          //
                          &iVal             ,          //
                          &compCode         ,          //
                          &reason          );          //
                                                       //
        setMqiItem( addMqiItem( anchor )  ,            //
                      selector            ,            //
                      INTIGER_ITEM        ,            //
                      (tMqiItemVal)(int)iVal );        //
        break;                                         //
      }                                                //
      // ---------------------------------------------------
      //  handle STRING item
      // ---------------------------------------------------
      case MQITEM_STRING :                             //
      {                                                //
        mqInquireString( bag               ,           //
                         MQSEL_ANY_SELECTOR,           //
                         i                 ,           //
                         ITEM_STRING_LENGTH,           //
                         itemStrBuff       ,           //
                         &itemStrLng       ,           //
                         &iCCSID           ,           //
                         &compCode         ,           //
                         &reason          );           //
                                                       //
        char* itemStr = (char*) malloc( sizeof(char) * // alloc string memory
                                        itemStrLng+1 );//
        if( itemStr == NULL )                          //
        {                                              //
          logger( LSTD_MEM_ALLOC_ERROR );              //
          goto _door;                                  //
        }                                              //
                                                       //
        memcpy( itemStr, itemStrBuff, itemStrLng );    // copy to str memory
        itemStr[itemStrLng] = '\0';                    //
                                                       //
        setMqiItem( addMqiItem( anchor ),              // add a new item
                      selector          ,              //  with bag value
                      STRING_ITEM       ,              //
                      (tMqiItemVal)(char*) itemStr );  //
        break;                                         //
      }                                                //
      case MQITEM_BAG :                                //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
      case MQITEM_BYTE_STRING :                        //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
      case MQITEM_INTEGER_FILTER :                     //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
      case MQITEM_STRING_FILTER :                      //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
      case MQITEM_INTEGER64 :                          //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
      case MQITEM_BYTE_STRING_FILTER :                 //
      {                                                //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) itemType,
                                              __FILE__, __LINE__ );
        sysRc = -3;                                    //
        goto _door;                                    //
      }                                                //
    }                                                  //
                                                       //
    switch( reason )                                   //
    {                                                  //
      case MQRC_NONE : break;                          //
      default :                                        //
      {                                                //
        logMQCall( DBG, "mqInquire???", reason );      //
        goto _door;                                    //
      }                                                //
    }                                                  //
  }                                                    //
                                                       //
  // -------------------------------------------------------
  // attach event list of items to the corresponding Queue Manager
  // -------------------------------------------------------
  qmgrItem = findMqiItem( anchor, MQCA_Q_MGR_NAME );   // find an item with 
  if( qmgrItem == NULL )                               //  queue manager name
  {                                                    //
    goto _door;                                        //
  }                                                    // attach event list of 
                                                       // items to the queue 
  qmgrNode = addQmgrNode( qmgrItem->value.strVal );    // manager node
                                                       //
  // -------------------------------------------------------
  // throw away item with queue manager name,  since name is already in the 
  // ---------------------------------------------------- header of the queue
  freeMqiItemValue( qmgrItem );                        // manager node 
  deleteMqiItem( anchor, qmgrItem );                   // 
                                                       //
  // -------------------------------------------------------
  // convert list of items to event list
  // -------------------------------------------------------
  sysRc = item2event( qmgrNode, anchor, pmd );         //
  if( sysRc < 0 )                                      //
  {                                                    //
    anchor = NULL;                                     //
    goto _door;                                        //
  }                                                    //
                                                       //
  _door:
	
  if(anchor) free(anchor);

//  freeItemList( anchor );
  return sysRc ;
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
  item->type         = UNKNOWN_ITEM;
  item->level        = MQEV_LEV_NA ;
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
/* find mqi item                                                              */
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
/* delete item                                                                */
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
/* free node value                                                            */
/******************************************************************************/
void freeMqiItemValue( tMqiItem *item )
{
  if( item->type == STRING_ITEM )
  {
    free( item->value.strVal );
  }
}

/******************************************************************************/
/* add qmgr node                                                              */
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
/* last queue manager node                                                    */
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
/* new event node                                                             */
/******************************************************************************/
tEvent* newEventNode()
{
  logFuncCall( ) ;
  tEvent *event; 

  event = (tEvent*) malloc( sizeof(tEvent));
  if( event == NULL )                          //
  {                                            //
    logger( LSTD_MEM_ALLOC_ERROR );            //
    goto _door;                                //
  }                                            //
                                               //
  event->item  = NULL;                          //
  event->next  = NULL;                          //
  event->level = MQEV_LEV_NA;
                                               //
  event->pmd = (PMQMD) malloc( sizeof(MQMD) ); //
  if( event->pmd == NULL )                     //
  {                                            //
    free(event);                               //
    event=NULL;                                //
    logger( LSTD_MEM_ALLOC_ERROR );            //
    goto _door;                                //
  }                                            //

  _door: 
  logFuncExit( ) ;
  return event;
}

/******************************************************************************/
/* add event node                                                             */
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
/* item to event                                                              */
/*                                                                            */
/*  description:                                                              */
/*    convert item list to event list                                      */
/*                                                                            */
/*  return code:                                                              */
/*     0: OK                                                                  */
/*     1: empty input (MQI-Item empty                                         */
/*    -1: missing code, message should go to the error queue                  */
/*    -2: missing code, message should go to the error queue              */
/*                                                                       */
/******************************************************************************/
int item2event( tQmgrNode *qmgrNode, tMqiItem *anchor, PMQMD pmd )
{
  logFuncCall( ) ;
 
  int sysRc = 0;
  tEvLevel level    ;

  tMqiItem *mqiItem;
  tMqiItem *nextMqiItem;
  tMqiItem *tmpItem;

  MQLONG eventType = 0 ;     // event type is value to MQIASY_COMMAND selector
                             //  - MQCMD_Q_MGR_EVENT 
                             //  - MQCMD_CHANNEL_EVENT
                             // 
  tEvent   *event  = newEventNode();
  if( event == NULL )        //
  {                          //
    goto _door ;             //
  }                          //
  event->item = NULL;        //
                             //
  if( anchor == NULL )       //
  {                          //
    sysRc = 1;               //
    goto _door;              //
  }                          //
                             //
  memcpy( event->pmd, pmd, sizeof(MQMD) );     // MQMD has been allocated 
  mqiItem = anchor->next;                      //  in newEventNode
                                               //
  // -------------------------------------------------------
  // go through all items, move them to event list, set the level
  // -------------------------------------------------------
  while( mqiItem )                             //
  {                                            //
#if(1)                                         //
    nextMqiItem = mqiItem->next;               //

    if( mqiItem->selector == MQIASY_COMMAND )  // find out to which list this
    {                                          //  item should be moved to
      eventType = mqiItem->value.intVal;       //  (single or double event)
    }                                          //
                                               // get level for selector, level
    level=getSelectorLevel(mqiItem->selector); //  of some selectors (with 
    if( level == MQEV_LEV_EVAL )               //  integer-value) depend on 
    {                                          //  it's value
      if( mqiItem->type != INTIGER_ITEM )      // converting is possible only 
      {                                        //  for integer items
	logger( LEVN_EVENT_ITEM_TYPE_ERROR,    //
	        mqSelector2str(mqiItem->selector));
        sysRc = -2;                            //
	goto _door;                            //
      }                                        //
      level = getValueLevel( mqiItem->value.intVal );
    }                                          //
                                               //
    switch( level )                            //
    {                                          //
      // ---------------------------------------------------
      // ignore this item,  remove item from the list, do not show this 
      // ---------------------------------------- information in the monitoring, 
      case MQEV_LEV_IGN:                       //
      {                                        //
        freeMqiItemValue( mqiItem );           // free item value for strings
        deleteMqiItem(anchor,mqiItem);         // delete and free item
        break;                                 //
      }                                        //
                                               //
      // ---------------------------------------------------
      // show following items in monitoring 
      // ---------------------------------------------------
      case MQEV_LEV_INF:                       // information ( green )
      case MQEV_LEV_WAR:                       // warning  ( yellow )
      case MQEV_LEV_ERR:                       // error    ( red )
      {                                        //
        mqiItem->level = level;                //
        moveMqiItem( mqiItem, anchor, event ); //
        break;                                 //
      }                                        //
                                               //
      // ---------------------------------------------------
      // evaluate level already done, this case should not be reached
      // ---------------------------------------------------
      case MQEV_LEV_EVAL:                      //
                                               //
      // ---------------------------------------------------
      // level not available, unknown level
      // ---------------------------------------------------
      case MQEV_LEV_NA:                        // return sysRc = -1
      default         :                        //  message should be moved to 
      {                                        //  an error queue
        logger(LEVN_MISSING_CODE_FOR_SELECTOR, //
                      (int) mqiItem->selector, // program will not abort in 
                      __FILE__,    __LINE__ ); //  case of missing code for some 
        freeItemList( event->item );           //  selector, it will just log 
        freeItemList( anchor );                //  an error message to an error 
        anchor = NULL;                         //  log and move MQ event message
        free( event->pmd );                    //  to an error queue
        free( event );                         //  (f.e in browseEvents 2 level
        event = NULL;                          //    higher in call stack)
        sysRc = -1;                            //
        goto _door;                            //
        break;                                 //
      }      /* default */                     //
    }        /* switch  */                     //
#else
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
        eventType = mqiItem->value.intVal; // at later stage in order to assign
//      freeMqiItemValue( mqiItem );       // stop/start qmgr events to a 
//      deleteMqiItem(anchor,mqiItem);     // special list 
        moveMqiItem( mqiItem, anchor, event );
        break;                             //
      }                                    //
      // ---------------------------------------------------
      // items to be moved in event list
      // ---------------------------------------------------
      case MQIASY_REASON    :              // reason code 
      case MQIACF_REASON_QUALIFIER    :    // reason qualifier
      // ---------------------------------------------------
      // Queue Manager Event
      // ---------------------------------------------------
      case MQIA_APPL_TYPE   :              // application type
      case MQCACF_APPL_NAME :              // application name
      case MQCA_Q_NAME      :              // queue name
      case MQCACF_USER_IDENTIFIER  :       // not authorized (connect)
      case MQIACF_OPEN_OPTIONS     :       // not authorized (open)
      case MQCA_BASE_OBJECT_NAME   :       // dlq reason
      case MQIA_BASE_TYPE          :       // dlq reason (combined with above)
      // ---------------------------------------------------
      // Channel events
      // ---------------------------------------------------
      case MQCACH_CHANNEL_NAME        :    // channel name
      case MQCACH_XMIT_Q_NAME         :    // transmission queue
      case MQCACH_CONNECTION_NAME     :    // conn name
      case MQIACF_ERROR_IDENTIFIER    :    // channel error
      case MQIACF_AUX_ERROR_DATA_INT_1:    // help error information
      case MQIACF_AUX_ERROR_DATA_INT_2:    // help error information
      case MQCACF_AUX_ERROR_DATA_STR_1:    // channel name
      case MQCACF_AUX_ERROR_DATA_STR_2:    // help error information
      case MQCACF_AUX_ERROR_DATA_STR_3:    // help error information
      {
        moveMqiItem( mqiItem, anchor, event );
        break;
      }
      
      // ---------------------------------------------------
      // missing selector in switch case list 
      // ---------------------------------------------------
      default :                                 // error handling including 
      {                                         // free of data structure
        logger( LEVN_MISSING_CODE_FOR_SELECTOR, //
                       (int) mqiItem->selector, // program will not abort in 
                       __FILE__,    __LINE__ ); //  case of missing code to some 
        freeItemList( event->item );            //  selector, it will just log 
        freeItemList( anchor );                 //  an error message and move MQ
        anchor = NULL;                          //  message to an error queue
        free( event->pmd );                     //
        free( event );                          //
        event = NULL;                           //
	sysRc = -1;                             //
        goto _door;                             //
      }                                         //
    }                                           //
    mqiItem = nextMqiItem;
#endif
             
    mqiItem = nextMqiItem;
  }
 
  // -------------------------------------------------------
  // evaluate event level (search for highest level in item list)
  // -------------------------------------------------------
  evalEventLevel( event );

  // -------------------------------------------------------
  // there should be exact one MQIASY_COMMAND item in the bag
  // evaluate it and move the event list to a single or double events
  // -------------------------------------------------------
  switch( eventType )
  {
    // -----------------------------------------------------
    // queue manager event 
    //   case message went to DLQ      -> put event to single event list
    //   case QMGR was stopped started -> put event to QMGR list
    // -----------------------------------------------------
    case MQCMD_Q_MGR_EVENT :                          //
    {                                                 //
      tmpItem=findMqiItem(event->item,MQIASY_REASON); // check the reason for
      if( tmpItem == NULL )            //
      {                                  //
	logger(LEVN_EVENT_ITEM_NOT_EXIST,"MQIASY_REASON");
        sysRc = -2;                  //
	goto _door;                    //
      }                              //
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
          addEventNode(qmgrNode->singleEvent,event);  //
          break;                                      //
        }                                             //
      }                                               //
      break;                                          // switch( eventType )
    }                                                 //
                                                  //
    // -----------------------------------------------------
    // channel event
    // -----------------------------------------------------
    case MQCMD_CHANNEL_EVENT:        //
    {                                //
      if( !(qmgrNode->qmgrEvent) )                //
      {                                               //
        qmgrNode->qmgrEvent = event;              //
        break;                                        //
      }                                               //
      addEventNode( qmgrNode->qmgrEvent, event ); //
      break;                                          //
    }                              //
                                              //
    // -----------------------------------------------------
    // not a queue manager event and 
    // not a channel event
    // -----------------------------------------------------
    default :                                         //
    {                                                 //
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) mqiItem->selector,
                                              __FILE__, __LINE__ );
      sysRc = -2;                                     //
      goto _door;                                     //
    }                                                 //
  }
  
  _door:
  logFuncExit( ) ;
  return sysRc ;
}


/******************************************************************************/
/*  move mqi item                                                             */
/*                                                                            */
/*    description:                                                            */
/*    items will be moved from the item list (anchor) to event list            */
/*                                                                            */
/******************************************************************************/

// this function does not work

void moveMqiItem( tMqiItem *actNode,  // item node to be moved
                  tMqiItem *anchor ,  // anchor of the source list
                  tEvent   *event  )  // parent (in tree) of the goal list
{
  logFuncCall( ) ;
  tMqiItem *itemNode = anchor;
  tMqiItem *lastEvItem = NULL ;
  
  while( itemNode->next != actNode )
  {
    itemNode = itemNode->next;
    if( !itemNode ) goto _door;
  }

  itemNode->next = actNode->next;
  actNode->next = NULL;
  if( !(event->item) )
  {
    event->item = actNode;
  }
  else
  {
    lastEvItem = lastMqiItem( event->item );
    lastEvItem->next = actNode;
  }
  
  _door:
          
  logFuncExit( ) ;
  return;
}

/******************************************************************************/
/*  get message id pairs                                                 */
/*                                                        */
/*    description:                                      */
/*      match close event and open event (f.e. stop/start queue manager or    */
/*      channel or queue depth high/low water               */
/*      up to MSGID_PAIR_AMOUNT/2 pairs are possible. This function will be   */
/*      recalled once more with the next main() cycle if there are more then  */
/*      MSGID_PAIR_AMOUNT event messages on the queue        */
/*                              */
/******************************************************************************/
PMQBYTE24 getMsgIdPair()
{
  logFuncCall( ) ;

  tQmgrNode *qmgrNode  = _gEventList; // go through all queue manager
  tEvent    *origEvent ;              // for each queue manager check all queues
  tEvent    *matchEvent;              // for each queue manager check all queues
  tMqiItem  *origItem  ;              // original item
  tMqiItem  *matchItem ;              // item that matches original item
  MQLONG     matchReason;             //
                                      //
  int msgIdCnt;                       //
                                      //
  // -------------------------------------------------------  
  // main loop, check all queue manager
  // -------------------------------------------------------  
#if(0)
  for( msgIdCnt=0; msgIdCnt<MSGID_PAIR_AMOUNT; msgIdCnt++ )
  {
    memcpy( _gMsgIdPair[msgIdCnt],' ',sizeof(MQBYTE24));
  }
#endif
  memcpy( _gMsgIdPair, MQMI_NONE , MSGID_PAIR_AMOUNT );

  msgIdCnt = 0;
                                              //
  while( qmgrNode )                           // check all queue manager
  {                                           //
    // -----------------------------------------------------  
    // handle original event
    // -----------------------------------------------------  
    origEvent = qmgrNode->qmgrEvent;          // get event list for every
                                              //  queue manager
    // -----------------------------------------------------  
    // go through all double events for actual queue manager
    // -----------------------------------------------------  
    while( origEvent )                        // go through all double events
    {                                         //
      origItem = findMqiItem( origEvent->item,// find item with a reason
  			      MQIASY_REASON );//
      if( !origItem )                         // original item does not include 
      {                                       // MQIASY_REASON, which should not
        logger( LEVN_EVENT_ITEM_NOT_EXIST ,   // appear. Still it would cause 
  	      "MQIASY_REASON"          );     // NULL Pointer exception
        goto _door;                           //
      }                                       //
      if( origItem->type != INTIGER_ITEM )    // item value for selector 
      {                                       // MQIASY_REASON, should always be
        logger( LEVN_EVENT_ITEM_TYPE_ERROR,   // integer, Still it would cause
  	      "MQIASY_REASON"          );     // type matching error later in 
        goto _door;                           // this function
      }                                       //
                                              // 
      // ---------------------------------------------------  
      // check if original event has a matching event
      // ---------------------------------------------------  
      matchReason = matchEventReason( origItem->value.intVal ); 
      if( matchReason == 0 )                  // MQIASY_REASON is a "single"
      {                                       //  event that does not match 
        origEvent = origEvent->next;          //  any start event 
        continue;                             // try next event
      }                                       //
                                              //
      // ---------------------------------------------------  
      // find match event
      // ---------------------------------------------------  
      matchEvent = origEvent->next;           // start event has to appear 
      while( matchEvent )                     //  after a stop event
      {                                       // check all rest events 
        matchItem=findMqiItem( matchEvent->item,
                               MQIASY_REASON);//
        if(matchItem                      &&  // original event matches some 
          matchItem->type == INTIGER_ITEM &&  //  start event
          matchItem->value.intVal == matchReason)
        {                                     //
          break;                              // break a searching loop
        }                                     //
        matchEvent = matchEvent->next;        //  
      }                                       //
                                              //
      // ---------------------------------------------------  
      // check existence of the match event and handle it
      // ---------------------------------------------------  
      if( matchEvent )                        //
      {                                       //
        memcpy( _gMsgIdPair[msgIdCnt]  ,      //
	        origEvent->pmd->MsgId  ,      //
	        sizeof(MQBYTE24)      );      //
        memcpy( _gMsgIdPair[msgIdCnt+1],      //
	        matchEvent->pmd->MsgId ,      //
	        sizeof(MQBYTE24)      );      //
        msgIdCnt += 2;                        //
        if( msgIdCnt > MSGID_PAIR_AMOUNT-1 )  //
        {                                     //
  	  goto _door;                         //
        }                                     //
      }                                       //
      origEvent = origEvent->next;            //
    }                                         //
                                              //
    qmgrNode = qmgrNode->next;                //
  }                                           //
                                              //
  _door:                                      //
  logFuncExit( ) ;
  return _gMsgIdPair ;

}

/******************************************************************************/
/*  match event reason                                                        */
/*                                                                            */
/*    description:                                                            */
/*      search a start selector to some stop selector                         */
/*      f.e. find start qmgr selector to stop qmgr selector                   */
/******************************************************************************/
MQLONG matchEventReason( MQLONG reason )
{
  logFuncCall( ) ;
  MQLONG match = 0 ;

  switch( reason )
  {
    // -----------------------------------------------------
    // queue manager stop / start
    // -----------------------------------------------------
    case MQRC_Q_MGR_NOT_ACTIVE:   // MQIASY_REASON
    case MQRQ_Q_MGR_STOPPING  :   // MQIACF_REASON_QUALIFIER
    case MQRQ_Q_MGR_QUIESCING :   // MQIACF_REASON_QUALIFIER
    {
      match = MQRC_Q_MGR_ACTIVE;  // MQIASY_REASON
      break;
    }
    default : break;
  }

  logFuncExit( ) ;
  return match;
}

/******************************************************************************/
/*  free event tree                                                           */
/*                                                                            */
/*    description:                                                            */
/*                                                                            */
/*      free complete event tree, do not free the list of the queue manager   */
/******************************************************************************/
void freeEventTree()
{
  tQmgrNode *qmgrNode = _gEventList ;

  if( qmgrNode == NULL ) goto _door ;

  while( qmgrNode )
  {
    freeItemTree( qmgrNode->qmgrEvent   );
    freeItemTree( qmgrNode->singleEvent );
    freeItemTree( qmgrNode->doubleEvent );
    qmgrNode->qmgrEvent   = NULL;
    qmgrNode->singleEvent = NULL;
    qmgrNode->doubleEvent = NULL;
    qmgrNode = qmgrNode->next ;
  }

  _door:
      return ;
}

/******************************************************************************/
/*  free item tree                                                            */
/******************************************************************************/
void freeItemTree( tEvent *_eventNode )
{
  tEvent *eventNode = _eventNode ;
  tEvent *nextNode  ;

  while( eventNode )
  {
    free( eventNode->pmd );
    freeItemList( eventNode->item );
    nextNode = eventNode->next;
    free( eventNode );
    eventNode = nextNode ; 
  }
}

/******************************************************************************/
/*  free item list                                                            */
/******************************************************************************/
void freeItemList( tMqiItem *_itemNode )
{
  tMqiItem *itemNode = _itemNode ;
  tMqiItem *nextNode ;

  while( itemNode )
  {
    freeMqiItemValue( itemNode );
    nextNode = itemNode->next ;
    free( itemNode );
    itemNode = nextNode ;
  }
}

/******************************************************************************/
/* find empty queue manager node                        */
/******************************************************************************/
#if(0)
char* findEmtpyQueueManager()
{
  tQmgrNode *node = *_gEventList = NULL;

  while( node )
  {
    if( node->doubleEvent == NULL &&
        node->qmgrEvent   == NULL &&
        node->singleEvent == NULL  )
    {
      break ;
    }
    node = node->next ;
  }

  if( node == NULL ) return NULL ;
  return node->qmgr ;
}
#endif


