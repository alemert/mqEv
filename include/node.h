/******************************************************************************/
/* change title on for new project                                            */
/******************************************************************************/

#ifndef  _MQ_EV_NODE_H
#define _MQ_EV_NODE_H
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

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <level.h>

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   S T R U C T S   AND   TYPES                     */
/******************************************************************************/

// ---------------------------------------------------------
// MQI item
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
  UNKNOWN_ITEM,
  INTIGER_ITEM,
  STRING_ITEM 
};

struct sMqiItem
{
  MQLONG     selector;
  tMqiItemVal   value ;
  tMqiItemType  type  ;
  tEvLevel      level ;
  tMqiItem     *next  ;
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
  tMqiItem *item;
  PMQMD     pmd ;
  tEvLevel level;
  tEvent   *next;
};

/******************************************************************************/
/*   G L O B A L E S                                                          */
/******************************************************************************/
#ifdef _MQEV_NODE_C_MODULE_
  tQmgrNode *_gEventList = NULL;
#else
  extern tQmgrNode *_gEventList ;
#endif

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int bag2mqiNode( PMQMD md, MQHBAG bag );
PMQBYTE24 getMsgIdPair();


tMqiItem* findMqiItem( tMqiItem* anchor , MQLONG selector );
//void freeMqiItemValue( tMqiItem *item );
void deleteMqiItem( tMqiItem* anchor, tMqiItem* deleteItem );

void freeEventTree();

// char* findEmtpyQueueManager() ;
#endif