/******************************************************************************/
/*   M Q   E V E N T   N O D E   U T I L I T I E S                         */
/*                                                                            */
/*   functions:                                                               */
/*    - bag2mqiNode                                                           */
/*    - newMqiItem                                                    */
/*    - addMqiItem                                                        */
/*    - lastMqiItem                                                  */
/*    - setMqiItem                                                  */
/*    - findMqiItem                                            */
/*    - deleteMqiItem                                  */
/*    - addQmgrNode                              */
/*    - findQmgrNode                                */
/*                                                  */
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

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define ITEM_STRING_LENGTH      64

/******************************************************************************/
/*   S T R U C T S   AND   TYPES         */
/******************************************************************************/

// ---------------------------------------------------------
// mqi item
// ---------------------------------------------------------
typedef struct sMqiItem     tMqiItem    ;
typedef union  uMqiItemVal  tMqiItemVal ;
typedef enum   eMqiItemType tMqiItemType;

union uMqiItemVal
{
  char *strVal;
  int   intVal;
};

enum eMqiItemType
{
  UNKNOWN,
  INTIGER,
  STRING
};

struct sMqiItem
{
  MQLONG     selector;
  tMqiItemVal   value   ;
  tMqiItemType  type    ;
  tMqiItem     *next    ;
};

// ---------------------------------------------------------
// qmgr node
// ---------------------------------------------------------
typedef struct sQmgrNode tQmgrNode;
typedef struct sEvent    tEvent   ;

struct sQmgrNode
{
  char qmgr[MQ_Q_MGR_NAME_LENGTH+1];
  tEvent    *qmgrEvent;
  tEvent    *singleEvent;
  tEvent    *doubleEvent;
  tQmgrNode *next;
};

struct sEvent
{
  MQLONG   selector;
  tMqiItem *item;
  tEvent   *next;
};

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
tQmgrNode *_gEventList = NULL;

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
void deleteMqiItem( tMqiItem* anchor, tMqiItem* deleteItem );

tQmgrNode* addQmgrNode( char* qmgrName );
tQmgrNode* findQmgrNode( char* qmgrName );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/
int bag2mqiNode( MQMD md, MQHBAG bag )
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

  tQmgrNode *qmgrNode ;
  
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
                      INTIGER             ,              //
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
                      STRING            ,                //
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
  if( qmgrItem == NULL )      //
  {            //
    goto _door;      //
  }          //
  qmgrNode = addQmgrNode( qmgrItem->value.strVal );      //
  deleteMqiItem( anchor, qmgrItem );      //
                //
  item2Event( qmgrNode, anchor );      //
             //
  _door:

  freeItemList( anchor );
  return 0;
}

/******************************************************************************/
/* new item node                                                              */
/******************************************************************************/
tMqiItem* newMqiItem( )
{
  logFuncCall( ) ;

  tMqiItem *item = (tMqiItem*) malloc( sizeof(tMqiItem) );

  if( item == NULL )
  {
    logger( LSTD_MEM_ALLOC_ERROR );
    goto _door;
  }  

  item->selector     = 0 ;
  item->value.strVal =  NULL ;
  item->type         = UNKNOWN ;
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

  if( anchor->next == NULL )
  {
    goto _door;
  }

  item = anchor->next;

  while( item )
  {
    if( item->selector == selector )
    {
      found = item;
      break; 
    }
  }
 
  _door:

  logFuncExit( ) ;
  return found ;
}

/******************************************************************************/
/* delete item            */
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

  item = anchor->next;

  while( item )
  {
    if( item->next == deleteItem )
    {
      item->next = deleteItem->next; 
      if( item->type == STRING )
      {
        free( item->value.strVal );
      }
      free( deleteItem);
      break; 
    }
  }
 
  _door:

  logFuncExit( ) ;
  return ;
}

/******************************************************************************/
/* add qmgr node                                */
/******************************************************************************/
tQmgrNode* addQmgrNode( char* qmgrName )
{
  logFuncCall( ) ;

  tQmgrNode *node;

  if( _gEventList == NULL ) 
  {
    _gEventList = (tQmgrNode*) malloc( sizeof(tQmgrNode) );
    node = _gEventList ;
    sprintf( node->qmgr, qmgrName );
    goto _door;
  }

  node = findQmgrNode( qmgrName );
  if( node ) 
  {
    goto _door;
  }
 
  node = (tQmgrNode*) malloc( sizeof(tQmgrNode) );
  sprintf( node->qmgr, qmgrName );
  node->next = NULL ;
  
  _door: 
  logFuncExit( ) ;
  return node;
}

/******************************************************************************/
/* find qmgr node                                              */
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

  while( node ) 
  {
    if( strcmp( qmgrName, node->qmgr ) == 0 )
    {
      found = node ;
      break ;
    }
  }

  _door: 
  logFuncExit( ) ;
  return found;
}
