/******************************************************************************/
/*   M Q   E V E N T   N O D E   U T I L I T I E S             */
/*                                          */
/*   functions:                    */
/*    - bag2node                      */
/*    - newItem              */
/*                          */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------

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
/*   G L O B A L S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define ITEM_STRING_LENGTH      64

/******************************************************************************/
/*   S T R U C T S   AND   TYPES         */
/******************************************************************************/

typedef struct sItem     tItem    ;
typedef union  uItemVal  tItemVal ;
typedef enum   eItemType tItemType;

#if(1)
union uItemVal
{
  char *strVal ;
  char *intVal ;
};

enum eItemType
{
  INTIGER,
  STRING
};

struct sItem
{
  MQLONG     selector;
  tItemVal   value   ;
  tItemType  type    ;
  tItem     *next    ;
};
#endif

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
tItem* newItem( );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/
int bag2node( MQMD md, MQHBAG bag )
{
  MQLONG itemCount;
  MQLONG itemType ;
  MQLONG selector ;

  MQINT32 iVal ;

  MQCHAR itemStrBuffer[ITEM_STRING_LENGTH+1];
  MQLONG itemStrLength;
  MQLONG iCCSID;

  MQLONG mqlongVal ;
  char  *mqstrVal  ;

  MQLONG compCode;
  MQLONG reason  ;
  int    i       ;

  tItem *anchor = newItemNode() ;
  
  // -------------------------------------------------------
  // count items in the bag
  // -------------------------------------------------------
  mqCountItems( bag                ,                 //
                MQSEL_ALL_SELECTORS,                 //
                &itemCount         ,                 //
                &compCode          ,                 //
                &reason           );                 //
                                                     //
  switch( reason )                                   // handle error count items
  {                                                  //
    case MQRC_NONE : break;                          //
    default :                                        //
    {                                                //
      logMQCall(DBG,"mqCountItems",reason);          //
      goto _door;                                    //
    }                                                //
  }                                                  //
  logger( LMQM_ITEM_COUNT, itemCount );              //
                                                     //
  // -------------------------------------------------------
  // go through all items
  // -------------------------------------------------------

  for( i=0; i<itemCount; i++ )                       // mqstrVal will stay NULL
  {                                                  // if item type is intiger
    mqstrVal = NULL;                                 // which can not be
                                                     // converted to string
    // -----------------------------------------------------
    // get item type
    // -----------------------------------------------------
    mqInquireItemInfo( bag               ,           // get selector(id)
                       MQSEL_ANY_SELECTOR,           //  and item type
                       i                 ,           //
                       &selector         ,           //
                       &itemType         ,           //
                       &compCode         ,           //
                       &reason          );           //
                                                     //
    switch( reason )                                 //
    {                                                //
      case MQRC_NONE : break;                        //
      default :                                      //
      {                                              //
        logMQCall(DBG,"mqCountItems",reason);        //
        goto _door;                                  //
      }                                              //
    }                                                //
    logger(LMQM_ITEM_TYPE,mqItemType2str(itemType)); //
                                                     //
    // -----------------------------------------------------
    // analyse the item type
    // -----------------------------------------------------
    switch( itemType )                               //
    {                                                //
      // ---------------------------------------------------
      //  handle INTIGER item
      // ---------------------------------------------------
      case MQITEM_INTEGER :                          //
      {                                              //
        mqInquireInteger( bag               ,        //
                          MQSEL_ANY_SELECTOR,        //
                          i                 ,        //
                          &iVal             ,        //
                          &compCode         ,        //
                          &reason          );        //
        mqlongVal = (MQLONG) iVal;                   //
        mqstrVal = (char*) itemValue2str(selector  , // try to convert to string
                                         mqlongVal); //
        break;                                       //
      }                                              //
      // ---------------------------------------------------
      //  handle STRING item
      // ---------------------------------------------------
      case MQITEM_STRING :                           //
      {                                              //
        mqInquireString( bag               ,         //
                         MQSEL_ANY_SELECTOR,         //
                         i                 ,         //
                         ITEM_STRING_LENGTH,         //
                         itemStrBuffer     ,         //
                         &itemStrLength    ,         //
                         &iCCSID           ,         //
                         &compCode         ,         //
                         &reason          );         //
        mqstrVal = (char*) itemStrBuffer   ;         //
        break;                                       //
      }                                              //
      case MQITEM_BAG :                              //
      {                                              //
        break;                                       //
      }                                              //
      case MQITEM_BYTE_STRING :                      //
      {                                              //
        break;                                       //
      }                                              //
      case MQITEM_INTEGER_FILTER :                   //
      {                                              //
        break;                                       //
      }                                              //
      case MQITEM_STRING_FILTER :                    //
      {                                              //
        break;                                       //
      }                                              //
      case MQITEM_INTEGER64 :                        //
      {                                              //
        break;                                       //
      }                                              //
      case MQITEM_BYTE_STRING_FILTER :               //
      {                                              //
        break;                                       //
      }                                              //
    }                                                //
                                                     //
    switch( reason )                                 //
    {                                                //
      case MQRC_NONE : break;                        //
      default :                                      //
      {                                              //
        logMQCall(DBG,"mqInquire???",reason);        //
        goto _door;                                  //
      }                                              //
    }                                                //

#if(0)                                                     //
    if( mqstrVal )                                   //
    {                                                //
      setDumpItemStr(  F_STR                  ,      //
                      mqSelector2str(selector),      //
                      mqstrVal               );      //
    }                                //
    else                      //
    {                                            //
      setDumpItemInt( F_MQLONG                ,      //
                      mqSelector2str(selector),  //
                      mqlongVal ) ;        //
    }                                            //
                            //
#endif
  }                                                //
                                                     //
  _door:
  return;
}

/******************************************************************************/
/* new item node            */
/******************************************************************************/
tItem* newItem( )
{
  logFuncCall( ) ;

  tItem *item = (tItem*) malloc( sizeof(tItem) );

  if( item == NULL )
  {
    logger( LSTD_MEM_ALLOC_ERROR );
    goto _door;
  }  

  selector->0;
  value   ->;
  type    ;
  item->next = NULL;
  _door:
 
  return item ;
 
  logFuncExit( ) ;
}
