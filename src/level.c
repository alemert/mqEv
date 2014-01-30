/******************************************************************************/
/*                              M Q   E V E N T                               */
/*                                                                            */
/*  description:                                                            */
/*    module for level handling;                               */
/*    for each selector should be a error level             */
/*                                                                        */
/*    functions:                                            */
/*      - getLevel                        */
/*                                          */
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
#include "cmdln.h"
#include <ctl.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <level.h>

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/
int getSelectorLevel( MQLONG selector )
{
  int level = MQEV_LEV_NA ;

  switch( selector )
  {
    // -----------------------------------------------------
    // system selectors to be ignored
    // -----------------------------------------------------
    case MQIASY_CODED_CHAR_SET_ID:     // -1, 
    case MQIASY_TYPE             :     // -2, MQCFT_EVENT, 
                                       //     other values should be an error
    case MQIASY_MSG_SEQ_NUMBER   :     // -4, ??? ???? ????
    case MQIASY_CONTROL          :     // -5, PCF Control Option
    case MQIASY_COMP_CODE        :     // -6, Completion code
    case MQIASY_BAG_OPTIONS      :     // -8, read only
    case MQIASY_VERSION          :     // -9,
    { 
      level = MQEV_LEV_IGN; 
      break; 
    }

    // -----------------------------------------------------
    // selectors depending on the value in the item
    // -----------------------------------------------------
    case MQIASY_REASON          :    //   -7, reason code, 
                                     //       the only evaluated system selector
    case MQIACF_REASON_QUALIFIER:    // 1020, reason qualifier
    case MQIACF_ERROR_IDENTIFIER:    // 1013, error identifier, (channel stopped 
    {                                //        by user is not an error)
      level = MQEV_LEV_EVAL; 
      break; 
    }

    // -----------------------------------------------------
    // informational selectors, queue manager event
    // -----------------------------------------------------
    case MQIA_APPL_TYPE   :              // application type
    case MQCACF_APPL_NAME :              // application name
    case MQCA_Q_NAME      :              // queue name
    {
      level = MQEV_LEV_INF; 
      break; 
    }

    // -----------------------------------------------------
    // informational selectors, channel event
    // -----------------------------------------------------
    case MQCACH_CHANNEL_NAME        :    // 3501, channel name
    case MQCACH_XMIT_Q_NAME         :    // 3505, transmission queue
    case MQCACH_CONNECTION_NAME     :    // 3506, conn name
    case MQIACF_AUX_ERROR_DATA_INT_1:    // 1070, help error information
    case MQIACF_AUX_ERROR_DATA_INT_2:    // 1071, help error information
    case MQCACF_AUX_ERROR_DATA_STR_1:    // 3026, channel name
    case MQCACF_AUX_ERROR_DATA_STR_2:    // 3027, help error information
    case MQCACF_AUX_ERROR_DATA_STR_3:    // 3028, help error information
    {
      level = MQEV_LEV_INF; 
      break; 
    }

    // -----------------------------------------------------
    // error selectors, queue manager event
    // -----------------------------------------------------
    case MQCACF_USER_IDENTIFIER  :       // not authorized (connect)
    case MQIACF_OPEN_OPTIONS     :       // not authorized (open)
    case MQCA_BASE_OBJECT_NAME   :       // dlq reason
    case MQIA_BASE_TYPE          :       // dlq reason (combined with above)
    {
      level = MQEV_LEV_ERR; 
      break; 
    }

    default : break;
  }

  return level;
}

