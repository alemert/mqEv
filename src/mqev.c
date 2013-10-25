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
#define COLLECTION_QUEUE    0
#define DONE_QUEUE          1
  

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int moveMessages( PMQBYTE24* msgIdArray, int getQueue, int putQueue );

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
/* browse events                          */
/******************************************************************************/
int browseEvents( )
{
  logFuncCall( ) ;

  int sysRc = 0 ;

  MQMD  evMsgDscr = {MQMD_DEFAULT};  // message descriptor (set to default)
  MQGMO getMsgOpt = {MQGMO_DEFAULT}; // get message option set to default
                                     //
  MQHBAG evBag;                      // 
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

  PMQBYTE24* msgIdPair ;

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

  MQMD  md  = {MQMD_DEFAULT} ;    // message descriptor (set to default)
  MQGMO gmo = {MQGMO_DEFAULT};    // get message option set to default
  MQPMO pmo = {MQPMO_DEFAULT};    // put message option set to default
                                  //
  static PMQVOID *buffer = NULL;  // message buffer
  static MQLONG  msgLng  = 512;   // message length
                                  //
  PMQBYTE24 msgId;                // array with all messages to moved
                                  //
  MQLONG reason;                  // mq reason
                                  //
  int sysRc = 0 ;                 // system reason
                                  //
  // -------------------------------------------------------  
  // initialize mq calls and open transaction
  // -------------------------------------------------------  
  if( !buffer )                           // message buffer has to be allocated 
  {                                       // on first call of this function
    buffer = (PMQVOID) malloc( msgLng );  //
  }                                       //
                                          //
  gmo.MatchOptions = MQMO_MATCH_MSG_TOKEN;// 
  gmo.Options      = MQGMO_CONVERT  +     //
                     MQGMO_SYNCPOINT;     //
  gmo.Version      = MQGMO_VERSION_3;     //
                        //
  md.Version = MQMD_VERSION_2;      //
                                          //
  sysRc = mqBegin( _ghConn );             // begin transaction
  switch( sysRc )                         //
  {                                       //
    case MQRC_NONE :                      //
    case MQRC_NO_EXTERNAL_PARTICIPANTS :  // transactions without external 
    {                                     //  resource manager
      break;                        //
    }                                //
    default : goto _door;                //
  }                                       //
                                          //
  msgId = msgIdArray[0];                  //
  while( *msgId != 0 )                    //
  {                                       //
    // -----------------------------------------------------  
    // read particular message
    // -----------------------------------------------------  
    memcpy( &(md.MsgId), msgId, sizeof(md.MsgId) );
                                          //
    reason = MQRC_NO_MSG_AVAILABLE;       // resizing the message buffer loop
    while( reason!=MQRC_NONE )            // resize message buffer if message
    {                                     //  truncated
      reason = mqGet( _ghConn   ,         // connection handle
                     _gohEvQueue,         // pointer to queue handle
                     buffer     ,         // message buffer
                     &msgLng    ,         // buffer length
                     &md        ,         // message Descriptor
                     gmo        ,         // get message option 
                     1         );         // wait interval in milli seconds
                                          //
      switch( reason )                    //
      {                                   //
        case MQRC_NONE : break;           // ok
        case MQRC_NO_MSG_AVAILABLE :      // this can only occur, if message is
        {                                 //  moved to acknowledge queue 
	  msgId++;                        //
          continue;                          //  manually at the same time
        }                                 //
        case MQRC_TRUNCATED_MSG_FAILED :  // message buffer to small for 
        {                                 //  the physical message, 
          logMQCall(WAR,"MQGET",reason);  //  resize (realloc) the buffer
          buffer = resizeMqMessageBuffer( buffer, &msgLng );
          continue;                       //
        }                                 //
        default :                         // real error (stopping qmgr)
	{                                 //
          sysRc = reason;              //
          goto _door;                     //
	}                                 //
      }                                   //
    }                                     //
                                          //
    // -----------------------------------------------------  
    // write the same message to done queue
    // -----------------------------------------------------  
    pmo.Options=MQPMO_FAIL_IF_QUIESCING ; //
    reason = mqPut( _ghConn     ,         // connection handle
                    _gohAckQueue,         // pointer to queue handle
                    &md         ,         // message descriptor
                    &pmo        ,         // Options controlling MQPUT
                    buffer      ,         // message buffer
                    msgLng     );         // message length (buffer length)
                                          //
    switch( reason )                      //
    {                                     //
      case MQRC_NONE : break;          //
      default :                           //
      {                                   //
        sysRc = reason;                //
        goto _door;                       //
      }                                   //
    }                                     //
    msgId++;        //
  }                                     //
                                    //
  // -------------------------------------------------------  
  // commit transaction
  // -------------------------------------------------------  
  mqCommit( _ghConn );                    //
  switch( sysRc )                         //
  {                                       //
    case MQRC_NONE : break;               //
    default        : goto _door;  //
  }                                       //
                                          //
  _door:
  logFuncExit( ) ;
  return sysRc ;
}