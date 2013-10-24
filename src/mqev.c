/******************************************************************************/
/*   M Q   E V E N T                                                          */
/*                                                                            */
/*   description:                                                             */
/*     central mq module handling mq event monitoring                    */
/*                                                                            */
/*   monitored events :                                                       */
/*     - Queue manager events (writing to SYSTEM.ADMIN.QMGR.EVENT)            */
/*        - Local Events      LOCALEV(ENABLED)                    */
/*        - Authority Events  AUTHOREV(ENABLED)                  */
/*        - Inibit Evnets     INHIBTEV(ENABLED)              */
/*                                  */
/*                                                                            */
/*                                                                            */
/*   functions:                                                      */
/*    - initMq                                              */
/*    - browseEvents                                      */
/*    - printEventList                        */
/*    - handleDoneEvents                    */
/*                                          */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <string.h>

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
#include <msgcat/lgstd.h>
#include <msgcat/lgmqm.h>

#include <inihnd.h>

#include <mqbase.h>
#include <mqev.h>

#include <node.h>

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
  MQHCONN _ghConn ;                      // global connection handle

  MQOD    _godEvQueue={MQOD_DEFAULT};    // global object description and
  MQHOBJ  _gohEvQueue;                   // object handler  for event queue

  MQOD    _godAckQueue = {MQOD_DEFAULT}; // global object description and
  MQHOBJ  _gohAckQueue;                  // object handler for acknowledge queue

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int moveMessages( PMQBYTE24 *msgIdArray, int getQueue, int putQueue );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* init mq connection                                          */
/******************************************************************************/
int initMq( )
{
  logFuncCall( ) ;

  MQLONG sysRc = MQRC_NONE ;

  // -------------------------------------------------------
  // connect to qmgr
  // -------------------------------------------------------
  char *qmgr = getStrAttr( "qmgr" );         // try to get qmgr name from cmdln
  if( qmgr == NULL )                         // if no qmgr name on cmdln 
  {                                          // try to get it from the ini file
     qmgr = getIniStrValue( getIniNode("mq","qmgr",NULL), "name" ); 
  }                                          //
                                             // if no qmgr on cmdln or ini
  sysRc = mqConn( qmgr, &_ghConn );          //   use default qmgr (qmgr==NULL)
  switch( sysRc )                            // connect to qmgr 
  {                                          //
    case MQRC_NONE : break ;                 // connect ok
    default        : goto _door ;            // connect failed
  }                                          //
                                             //
  // -------------------------------------------------------
  // open event queue
  // -------------------------------------------------------
  char *evQueue = getStrAttr( "queue" );   // get event queue name from cmdln
  if( evQueue == NULL )                    // if not found, get it from ini file
  {                                        //
    evQueue = getIniStrValue( getIniNode("mq","qmgr","evqueue",NULL), "name" ); 
  }                                        //
                                           //
  if( evQueue == NULL )                    // if not found on command line and 
  {                                        // not in ini file abort process
    logger( LSTD_UNKNOWN_CMDLN_ATTR,"queue");
    logger( LSTD_UNKNOWN_INI_ATTR  ,       //
	   "mq - qmgr - evqueue - name" ); //
    logger( LSTD_GEN_ERR, __FUNCTION__ );  //
    sysRc = 1 ;                            //
    goto _door;                            //
  }                                        // set queue name in object 
                                           //  description structure
  memset(_godEvQueue.ObjectName,(int) ' ',MQ_Q_NAME_LENGTH);
  memcpy(_godEvQueue.ObjectName,evQueue  ,MQ_Q_NAME_LENGTH);
                                           //
  sysRc = mqOpenObject(                    //
              _ghConn                ,     // qmgr connection handle 
              &_godEvQueue           ,     // event q object descriptor 
              MQOO_INPUT_AS_Q_DEF    |     //   open object for get
              MQOO_BROWSE            |     //   open for browse
              MQOO_FAIL_IF_QUIESCING ,     //   open fails if qmgr is stopping
              &_gohEvQueue          );     // object handle event queue
                                           //
  switch( sysRc )                          // rc mqopen
  {                                        //
    case MQRC_NONE : break;                // open ok
    default        : goto _door ;          // open failed
  }                                        //
                                           //
  // -------------------------------------------------------
  // open acknowledge queue
  // -------------------------------------------------------
  char* ackQueue = getIniStrValue(         //
                     getIniNode( "mq","qmgr","ackqueue",NULL), "name" ); 
                                           //
  if( ackQueue == NULL )                   // if acknowledge queue name not 
  {                                        // found in ini file abort process
    logger( LSTD_UNKNOWN_INI_ATTR,         //
	   "mq - qmgr - ackqueue - name" );//
    logger( LSTD_GEN_ERR, __FUNCTION__ );  //
    sysRc = 1 ;                            //
    goto _door;                            //
  }                                        // set queue name in object 
                                           //  description structure
  memset(_godAckQueue.ObjectName,(int) ' ',MQ_Q_NAME_LENGTH);
  memcpy(_godAckQueue.ObjectName,evQueue  ,MQ_Q_NAME_LENGTH);
                                           //
  sysRc = mqOpenObject(                    //
              _ghConn                ,     // qmgr connection handle 
              &_godAckQueue          ,     // event q object descriptor 
              MQOO_INPUT_AS_Q_DEF    |     //   open object for get
              MQOO_BROWSE            |     //   open for browse
              MQOO_FAIL_IF_QUIESCING ,     //   open fails if qmgr is stopping
              &_gohAckQueue         );     // object handle event queue
                                //
  switch( sysRc )                          // rc mqopen
  {                                        //
    case MQRC_NONE : break;                // open ok
    default        : goto _door ;          // open failed
  }                                        //
                                  //
  _door :

  logFuncExit( ) ;

  return sysRc ;
}

/******************************************************************************/
/* browse events                    */
/******************************************************************************/
int browseEvents( )
{
  logFuncCall( ) ;

  int sysRc = 0 ;

  MQMD  evMsgDscr = {MQMD_DEFAULT};  // message descriptor (set to default)
  MQGMO getMsgOpt = {MQGMO_DEFAULT}; // get message option set to default
                                     //
  MQHBAG evBag;              // 
                                     //
//MQLONG  compCode;                  //
  MQLONG  reason  = MQRC_NONE;       //
                                     //
  sysRc = reason;                    //
                                     //
  // -------------------------------------------------------  
  // init mq for get events
  // -------------------------------------------------------  
  getMsgOpt.Options     |= MQGMO_BROWSE_NEXT; // browse messages
  getMsgOpt.MatchOptions = MQMO_NONE;         //
                                              //
  reason = mqOpenBag( &evBag );               //
                                              //
  if( reason != MQRC_NONE )                   //
  {                                           //
    sysRc = reason;                           //
    goto _door;                               //
  }                                           //
                                              //
  // -------------------------------------------------------  
  // browse available events 
  // -------------------------------------------------------  
  dumpMqStruct( "GMO  ", &getMsgOpt, NULL );  //
  dumpMqStruct( "MD   ", &evMsgDscr, NULL );  //
                                              //
  while( reason != MQRC_NO_MSG_AVAILABLE )    // browse all available messages
  {                                           //
    reason = mqReadBag( _ghConn    ,          // global (qmgr) connect handle
                        _gohEvQueue,          // globale (queue) open handle
                        &evMsgDscr ,          // message descriptor
                        &getMsgOpt ,          // get message options
                         evBag    );          // bag
                                              //
    switch( reason )                          //
    {                                         //
      case MQRC_NONE :                        //
      {                                       //
        bag2mqiNode( &evMsgDscr, evBag );     //
        continue;                             //
      }                                       //
      case MQRC_NO_MSG_AVAILABLE :            //
      {                                       //
        continue;                             //
      }                                       //
      default :                               //
      {                                       //
        sysRc = reason;                       //
        goto _door;                           //
      }                                       //
    }                                         //
                                              //
  }                                           //
                                              //
  // -------------------------------------------------------  
  // delete (opened) bag 
  // -------------------------------------------------------  
  reason = mqCloseBag( &evBag );              // mqGetBag interface
                                              //
  if( reason != MQRC_NONE )                   // handle mqCreateBag error
  {                                           //
    sysRc = reason;                           //
    goto _door;                               //
  }                                           //

  _door:

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*  handle done events                    */
/******************************************************************************/
int handleDoneEvents()
{
  logFuncCall( ) ;
  int sysRc = 0 ;

  PMQBYTE24 msgIdPair ;

  msgIdPair = getMsgIdPair();
  
  moveMessages( msgIdPair, COLLECTION_QUEUE, DONE_QUEUE );

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*   move messages                              */
/******************************************************************************/
int moveMessages( PMQBYTE24 *msgIdArray, int getQueue, int putQueue )
{
  logFuncCall( ) ;

  PMQBYTE24 *msgId;
  int sysRc = 0 ;

  sysRc = mqBegin( _ghConn );                 // qmgr connection handle 
  switch( sysRc )
  {
    case MQRC_NONE : break;
    default        : goto _door;
  }

  msgId = msgIdArray ;
  while( *msgId != 0 )
  {
    // get
    // put
  }
 
  mqCommit( _ghConn );
  switch( sysRc )
  {
    case MQRC_NONE : break;
    default        : goto _door;
  }

  _door:
  logFuncExit( ) ;
  return sysRc ;
}