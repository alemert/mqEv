/******************************************************************************/
/*                              M Q   E V E N T                               */
/*                                                                            */
/*  description:                                                              */
/*    includes for level handling;                                            */
/*                                                                            */
/******************************************************************************/

#ifndef  _MQ_EV_LEVEL_H
#define _MQ_EV_LEVEL_H
/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/
// ---------------------------------------------------------
// system
// ---------------------------------------------------------

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#if(0)
#define MQEV_LEV_EVAL   0      // evaluate the level depending on item value
#define MQEV_LEV_IGN    1      // ignore( do not show the message)
#define MQEV_LEV_INF    2      // information ( green )
#define MQEV_LEV_WAR    3      // warning  ( yellow )
#define MQEV_LEV_ERR    4      // error    ( red )
#define MQEV_LEV_NA     5      // level not available move message to an error 
                               //  queue, log error to a log file
#endif

/******************************************************************************/
/*   G L O B A L E S                                                          */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   S T R U C T S   AND   TYPES                           */
/******************************************************************************/
typedef enum eEvLevel tEvLevel;

enum eEvLevel 
{
  MQEV_LEV_EVAL = 0,   // evaluate the level depending on item value
  MQEV_LEV_IGN  = 1,   // ignore( do not show the message)
  MQEV_LEV_INF  = 2,   // information ( green )
  MQEV_LEV_WAR  = 3,   // warning  ( yellow )
  MQEV_LEV_ERR  = 4,   // error    ( red )
  MQEV_LEV_NA   = 5    // level not available move message to an error 
                       //  queue, log error to a log file
};

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int getSelectorLevel( MQLONG selector );


#endif    // !_MQ_EV_LEVEL_H