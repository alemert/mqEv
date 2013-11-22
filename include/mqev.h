/******************************************************************************/
/*   M Q   E V E N T                                                          */
/*                                                            */
/*   central header files for the project                            */
/*                                                                        */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/
// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <cmqc.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------


/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   G L O B A L E S                                                          */
/******************************************************************************/
#ifndef _MQEV_MQEV_C_MODULE_
  extern MQHCONN _ghConn ;                          
                                             
  extern MQOD    _godEvQueue;
  extern MQHOBJ  _gohEvQueue;
                                            
  extern MQOD    _godAckQueue;
  extern MQHOBJ  _gohAckQueue;
                                           
  extern MQOD    _godStoreQueue;
  extern MQHOBJ  _gohStoreQueue;
#endif

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int initMq( );
void endMq( );
int browseEvents( MQHOBJ _ohQ );
void printEventList();
int handleDoneEvents();
void msgIdStr2MQbyte( char* str, PMQBYTE24 msgid );
MQLONG acknowledgeMessages();
MQLONG acceptMessages();


