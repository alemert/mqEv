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
/*                                                    */
/*                                                                            */
/*                                                                            */
/*   functions:                                                               */
/*    - initMq                                                              */
/*    - browseEvents                                                      */
/*    - printEventList                                        */
/*    - handleDoneEvents                                  */
/*    - moveMessages                                */
/*    - msgIdStr2MQbyte                            */
/*    - acknowledgeMessages                      */
/*    - acceptMessages                    */
/*                                          */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
// ---------------------------------------------------------
// local
// ---------------------------------------------------------

#define _MQEV_MQEV_C_MODULE_
#include <mqev.h>

#include <node.h>
#include <process.h>


/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
  MQHCONN _ghConn ;                          // global connection handle
                                             //
  MQOD    _godEvQueue={MQOD_DEFAULT};        // global object description and
  MQHOBJ  _gohEvQueue=MQHO_UNUSABLE_HOBJ;    // object handler  for event queue
                                             //
  MQOD    _godAckQueue={MQOD_DEFAULT};       // global object description and
  MQHOBJ  _gohAckQueue=MQHO_UNUSABLE_HOBJ;   // object handler for acknowledge 
                                             //  queue
  MQOD    _godStoreQueue={MQOD_DEFAULT};     // global object description and
  MQHOBJ  _gohStoreQueue=MQHO_UNUSABLE_HOBJ; // object handler for store queue 
                                             //
                                             //
/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/
#define INITIAL_MESSAGE_LENGTH      512

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int moveMessages( PMQBYTE24 msgIdArray, int getQueue, int putQueue );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* init mq connection                                                         */
/******************************************************************************/
int initMq( )
{
  logFuncCall( ) ;

  MQLONG sysRc = MQRC_NONE ;

  // -------------------------------------------------------
  // connect to qmgr
  // -------------------------------------------------------
  char *qmgr = getStrAttr( "qmgr" );       // try to get qmgr name from cmdln
  if( qmgr == NULL )                       // if no qmgr name on cmdln 
  {                                        // try to get it from the ini file
     qmgr = getIniStrValue( getIniNode("connect","qmgr",NULL), "name" ); 
  }                                        //
                                           //
  if( qmgr == NULL )                       //
  {                                        //
    logger( LSTD_UNKNOWN_CMDLN_ATTR,"qmgr" );
    logger( LSTD_UNKNOWN_INI_ATTR    ,     //
	    "connect - qmgr - name" );     //
  }                                        //
                                           // if no qmgr on cmdln or ini
  sysRc = mqConn( qmgr, &_ghConn );        //   use default qmgr (qmgr==NULL)
  switch( sysRc )                          // connect to qmgr 
  {                                        //
    case MQRC_NONE : break ;               // connect ok
    default        : goto _door ;          // connect failed
  }                                        //
                                           //
  // -------------------------------------------------------
  // open event queue
  // -------------------------------------------------------
  char *evQueue = getStrAttr( "queue" );   // get event queue name from cmdln
  if( evQueue == NULL )                    // if not found, get it from ini file
  {                                        //
    evQueue = getIniStrValue( getIniNode( "connect", "queue",
					  "collect", NULL)  , 
			                  "name" ); 
  }                                        //
                                           //
  if( evQueue == NULL )                    // if not found on command line and 
  {                                        // not in ini file abort process
    logger( LSTD_UNKNOWN_CMDLN_ATTR_ERR,"queue");
    logger( LSTD_UNKNOWN_INI_ATTR_ERR  ,   //
	   "connect - queue - collect - name" ); 
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
  // open store queue
  // -------------------------------------------------------
  char* storeQueue = getIniStrValue( getIniNode( "connect", "queue",
					         "store"  , NULL  ), 
				                 "name"  ); 
                                           //
  if( storeQueue == NULL )                 // if store queue name not 
  {                                        // found in ini file abort process
    logger( LSTD_UNKNOWN_INI_ATTR_ERR,     //
	   "collect - queue - store - name" );
    logger( LSTD_GEN_ERR, __FUNCTION__ );  //
    sysRc = 1 ;                            //
    goto _door;                            //
  }                                        // set queue name in object 
                                           //  description structure
  memset(_godStoreQueue.ObjectName,(int) ' '  ,MQ_Q_NAME_LENGTH);
  memcpy(_godStoreQueue.ObjectName,storeQueue ,MQ_Q_NAME_LENGTH);
                                           //
  sysRc = mqOpenObject(                    //
              _ghConn                ,     // qmgr connection handle 
              &_godStoreQueue        ,     // event q object descriptor 
              MQOO_INPUT_AS_Q_DEF    |     //   open object for get
              MQOO_OUTPUT            |     //   open object for get
              MQOO_BROWSE            |     //   open for browse
              MQOO_SET_ALL_CONTEXT   |     //   keep original date/time im MQMD
              MQOO_FAIL_IF_QUIESCING ,     //   open fails if qmgr is stopping
              &_gohStoreQueue       );     // object handle event queue
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
  char* ackQueue = getIniStrValue( getIniNode( "connect"    , "queue",
					       "acknowledge", NULL  ), 
				               "name" ); 
                                           //
  if( ackQueue == NULL )                   // if acknowledge queue name not 
  {                                        // found in ini file abort process
    logger( LSTD_UNKNOWN_INI_ATTR_ERR,     //
	   "connect - queue - acknowledge - name" );
    logger( LSTD_GEN_ERR, __FUNCTION__ );  //
    sysRc = 1 ;                            //
    goto _door;                            //
  }                                        // set queue name in object 
                                           //  description structure
  memset(_godAckQueue.ObjectName,(int) ' ',MQ_Q_NAME_LENGTH);
  memcpy(_godAckQueue.ObjectName,ackQueue ,MQ_Q_NAME_LENGTH);
                                           //
  sysRc = mqOpenObject(                    //
              _ghConn                ,     // qmgr connection handle 
              &_godAckQueue          ,     // event q object descriptor 
              MQOO_OUTPUT            |     //   open object for get
              MQOO_BROWSE            |     //   open for browse
              MQOO_SET_ALL_CONTEXT   |     //   keep original date/time im MQMD
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
/*  end MQ                                                                    */
/******************************************************************************/
void endMq()
{
  logFuncCall( ) ;

  if( _gohEvQueue != MQHO_UNUSABLE_HOBJ )      // close event queue if open
  {                                            //
    mqCloseObject( _ghConn, &_gohEvQueue );    // ignore reason, end of program
  }                                            //
                                               //
  if( _gohAckQueue != MQHO_UNUSABLE_HOBJ )     // close acknowledge queue if
  {                                            //  open
    mqCloseObject( _ghConn, &_gohAckQueue );   // ignore reason, end of program
  }                                            //
                                               //
  if( _gohStoreQueue != MQHO_UNUSABLE_HOBJ )   // close store queue if open
  {                                            //
    mqCloseObject( _ghConn, &_gohStoreQueue ); // ignore reason, end of program
  }                                            //
                                               //
  mqDisc( &_ghConn ) ;                         // global connection handle
                                               //
  logFuncExit( ) ;
}

/******************************************************************************/
/* browse events                                                              */
/******************************************************************************/
int browseEvents( MQHOBJ _ohQ )
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
  int firstBrowse = 1 ;              //
                                     //
  sysRc = reason;                    //
                                     //
  // -------------------------------------------------------  
  // init mq for get events
  // -------------------------------------------------------  
  getMsgOpt.Options     |= MQGMO_BROWSE_FIRST;// browse messages
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
                        _ohQ       ,          // globale (queue) open handle
                        &evMsgDscr ,          // message descriptor
                        &getMsgOpt ,          // get message options
                         evBag    );          // bag
                                              //
    if( firstBrowse )                         //
    {                                         //
      getMsgOpt.Options -= MQGMO_BROWSE_FIRST;// 
      getMsgOpt.Options |= MQGMO_BROWSE_NEXT; // 
      firstBrowse = 0;                        //
    }                                         //
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
        break;                                //
      }                                       //
      default :                               //
      {                                       //
        sysRc = reason;                       //
        goto _door;                           //
      }                                       //
    }                                         //
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
/*  handle done events                                                        */
/******************************************************************************/
int handleDoneEvents()
{
  logFuncCall( ) ;
  int cnt;
  int sysRc = 0 ;

  PMQBYTE24 msgIdPair ;

  msgIdPair = getMsgIdPair();
  
  sysRc = moveMessages( msgIdPair, _gohStoreQueue, _gohAckQueue );
  if( sysRc != 0 )
  {
    goto _door;
  }

  if( sysRc == 0 )
  {
    cnt=0;
    while( memcmp(msgIdPair[cnt],MQMI_NONE,sizeof(MQBYTE24)) != 0 )
    {
      cnt++; 
    }
    sysRc = -cnt;
  }

  _door:

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*   move messages                                                            */
/******************************************************************************/
int moveMessages( PMQBYTE24 _msgIdArray, MQHOBJ _getoh, MQHOBJ _putoh )
{
  logFuncCall( ) ;

  MQMD  md  = {MQMD_DEFAULT} ;        // message descriptor (set to default)
  MQGMO gmo = {MQGMO_DEFAULT};        // get message option set to default
  MQPMO pmo = {MQPMO_DEFAULT};        // put message option set to default
                                      //
  static PMQVOID *buffer = NULL;      // message buffer
  static MQLONG  msgLng  = INITIAL_MESSAGE_LENGTH;    // message length
  static MQLONG  bufferSize=INITIAL_MESSAGE_LENGTH; // maximal message length
                                      //
  PMQBYTE24 msgId;                    // array with all messages to moved
                                      //
  MQLONG reason;                      // mq reason
                                      //
  int sysRc = 0 ;                     // system reason
                                      //
  // -------------------------------------------------------  
  // initialize mq calls and open transaction
  // -------------------------------------------------------  
  if( !buffer )                           // message buffer has to be allocated 
  {                                       // on first call of this function
    buffer = (PMQVOID) malloc( msgLng );  //
  }                                       //
                                          //
  gmo.MatchOptions = MQMO_MATCH_MSG_ID;   // 
  gmo.Options      = MQGMO_CONVERT  +     //
                     MQGMO_SYNCPOINT+     //
                     MQGMO_NO_WAIT  ;     //
  gmo.Version      = MQGMO_VERSION_3;     //
                                          //
  md.Version  = MQMD_VERSION_2;            //
  md.Encoding = MQENC_NATIVE ;
  md.CodedCharSetId = MQCCSI_Q_MGR;
                                          //
  sysRc = mqBegin( _ghConn );             // begin transaction
  switch( sysRc )                         //
  {                                       //
    case MQRC_NONE :                      //
    case MQRC_NO_EXTERNAL_PARTICIPANTS :  // transactions without external 
    {                                     //  resource manager
      break;                              //
    }                                     //
    default : goto _door;                 //
  }                                       //
                                          //
  msgId = _msgIdArray ;                   //
  while( memcmp( msgId, MQMI_NONE, MQ_MSG_ID_LENGTH ) != 0 ) 
  {                                       //
    // -----------------------------------------------------  
    // read particular message
    // -----------------------------------------------------  
                                          //
    memcpy( &(md.MsgId), msgId, MQ_MSG_ID_LENGTH );
    int loop = 1;                         // resizing the message buffer loop
    while( loop == 1 )                    // resize message buffer if message
    {                                     //  truncated
      reason = mqGet( _ghConn,           // connection handle
                      _getoh ,           // pointer to queue handle
                      buffer ,           // message buffer
                      &msgLng,           // buffer length
                      &md    ,           // message Descriptor
                      gmo    ,           // get message option 
                      1     );           // wait interval in milli seconds
                                          //
      switch( reason )                    //
      {                                   //
        case MQRC_NONE :                  //
	{                                 //
          loop = 0;                       //
	  break;                          // ok
	}                                 //
        case MQRC_NO_MSG_AVAILABLE :      // this can only occur, if message is
        {                                 //  moved to acknowledge queue 
	  loop = -1;                      //
          break;                          //  manually at the same time
        }                                 //
        case MQRC_TRUNCATED_MSG_FAILED :  // message buffer to small for 
        {                                 //  the physical message, 
          logMQCall(WAR,"MQGET",reason);  //  resize (realloc) the buffer
	  bufferSize = msgLng ;           //
          buffer = resizeMqMessageBuffer( buffer, &bufferSize );
          continue;                       //
        }                                 //
        default :                         // real error (stopping qmgr)
	{                                 //
          sysRc = reason;                 //
          goto _door;                     //
	}                                 //
      }                                   //
    }                                     //
    if( loop == -1 )                      //
    {                                     //
      msgId++;                            //
      continue;                           //
    }                                     //
                                          //
    // -----------------------------------------------------  
    // write the same message to done queue
    // -----------------------------------------------------  
    pmo.Options=MQPMO_FAIL_IF_QUIESCING + //
	        MQPMO_NO_CONTEXT        + //
                MQPMO_SYNCPOINT         ; //
    reason = mqPut( _ghConn     ,         // connection handle
                    _putoh      ,         // pointer to queue handle
                    &md         ,         // message descriptor
                    &pmo        ,         // Options controlling MQPUT
                    buffer      ,         // message buffer
                    msgLng     );         // message length (buffer length)
                                          //
    switch( reason )                      //
    {                                     //
      case MQRC_NONE : break;             //
      default :                           //
      {                                   //
        sysRc = reason;                   //
        goto _door;                       //
      }                                   //
    }                                     //
    msgId++;                              //
  }                                       //
                                          //
  // -------------------------------------------------------  
  // commit transaction
  // -------------------------------------------------------  
  mqCommit( _ghConn );                    //
  switch( sysRc )                         //
  {                                       //
    case MQRC_NONE : break;               //
    case MQRC_NO_EXTERNAL_PARTICIPANTS:   // MQ only, no external participants 
    {                                     //  to avoid evaluating RC in calling
      sysRc = MQRC_NONE;                  // functions set sysRc to 0
      break;                              //
    }                                     //
    default : goto _door;                 //
  }                                       //
                                          //
  _door:
  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*  message id string to MQBYTE24                                             */
/******************************************************************************/
void msgIdStr2MQbyte( char* _str, PMQBYTE24 _pmsgid )
{
  logFuncCall( ) ;

  char buff[3];
  char *str;
  int i ;

  MQBYTE24 msgid ;

  if( memcmp( _str, "0x", 2 ) == 0 )
  {
    _str += 2 ;
  }
  memset( msgid, 0, sizeof(MQBYTE24) );
  for( i=0, str=_str; i<24; i++)
  {
    memcpy( buff, str, 2 );
    buff[2] = '\0' ;
    msgid[i] = (MQBYTE) strtoul( buff, NULL, 16);
    str += 2;
  }

  memcpy( _pmsgid, &msgid, sizeof(MQBYTE24) );
  logFuncExit( ) ;
}

/******************************************************************************/
/*  acknowledge messages                                                      */
/******************************************************************************/
MQLONG acknowledgeMessages( )
{
  logFuncCall( ) ;
  MQLONG sysRc ;

  char** msgIdArr    ;
  int    msgIdArrSize;

  PMQBYTE24 msgId24 = NULL ;

  int i;

  // -------------------------------------------------------
  // get message id's from command line as a string, 
  //   allocate memory for message id's in hex format
  //   convert string into hex
  //   move messages from store queue to confirm queue
  // -------------------------------------------------------
  msgIdArr = getStrArrayAttr( "ack" );      // get all message id's from 
  if( !msgIdArr )                           //   the command line
  {                                         // this has already been checked in 
    goto _door;                             //   main(), message id is not NULL
  }                                         //
                                            //
  msgIdArrSize = getAttrSize( "ack" );      // get nr. of message id's
                                            //
  msgId24 = (PMQBYTE24) malloc( sizeof(MQBYTE24)*(msgIdArrSize+1) );
  if( !msgId24 )                            // allocate memory for message id's
  {                                         //
    logger( LSTD_MEM_ALLOC_ERROR );         //
    goto _door;                             //
  }                                         //
                                            //
  for( i=0; i< msgIdArrSize; i++ )          // convert numerical message id 
  {                                         //   to string
    msgIdStr2MQbyte( msgIdArr[i], (msgId24+i) );  
  }                                         //
  memcpy( (msgId24+1), MQMI_NONE, sizeof(MQBYTE24) );
                                            //
  sysRc = moveMessages( msgId24      ,      // move messages from store to 
                       _gohStoreQueue,      // confirm queue 
                       _gohAckQueue );      // move messages handles whole 
  if( sysRc != 0 )                          // transaction including commit
  {                                         // 
    goto _door;                             //
  }                                         //
                                            //
  _door :                                   // 
                                            //
  if( msgId24) free( msgId24 );             //
                                            //
  logFuncExit( );
  return sysRc  ;
}

/******************************************************************************/
/*  accept messages                                                           */
/*                  */
/*    move messages from collection queue to store queue                      */
/******************************************************************************/
MQLONG acceptMessages( char _movedMsgQmgr[TRANSACTION_SIZE][MQ_Q_MGR_NAME_LENGTH+1] )
{
  logFuncCall( ) ;
  MQLONG sysRc ;

  MQLONG reason;                   //
                                   //
  MQMD  md  = {MQMD_DEFAULT} ;     // message descriptor (set to default)
  MQGMO gmo = {MQGMO_DEFAULT};     // get message option set to default
  MQPMO pmo = {MQPMO_DEFAULT};     // put message option set to default
                                   //
  static PMQVOID *buffer = NULL;   // message buffer
  static MQLONG  msgLng  = INITIAL_MESSAGE_LENGTH;    // message length
  static MQLONG  bufferSize = INITIAL_MESSAGE_LENGTH; // maximal message length
                                   //
  int msgCnt;                      // maximal transaction size
  int transSize;                   // transSize
  int *pIni;                       // pointer for receiving int ini values
                                   //
  tIniNode *searchIni ;            // data structure for searching in ini files
                                   //
  int movedMessages=0;             //
  int i;                           //
  int j;                           //
                                   //
  // -------------------------------------------------------  
  // init vara
  // -------------------------------------------------------  
  memset( _movedMsgQmgr, 0, MQ_Q_MGR_NAME_LENGTH * 
                            TRANSACTION_SIZE     * 
                            sizeof(char)        );

  // -------------------------------------------------------  
  // initialize mq calls and open transaction
  // -------------------------------------------------------  
  searchIni = getIniNode("mq", "transaction");// find mq.transaction ini-node 
  pIni = getIniIntValue(searchIni,"size");    // get transaction size
  if( !pIni )                                 //
  {                                           //
    transSize = TRANSACTION_SIZE;             //
  }                                           //
  else                                        //
  {                                           //
    transSize = *pIni;                        //
  }                                           //
  msgCnt = transSize;                         //
                                              //
  if( !buffer )                               // message buffer has to be 
  {                                           // allocated at first call of 
    buffer = (PMQVOID) malloc( msgLng );      // this function
  }                                           //
                                              //
  gmo.MatchOptions = MQMO_MATCH_MSG_ID;       // 
  gmo.Options      = MQGMO_CONVERT    +       //
                     MQGMO_SYNCPOINT  ,       //
                     MQGMO_WAIT       ;       //
  gmo.Version      = MQGMO_VERSION_3  ;       //
                                              //
  md.Version = MQMD_VERSION_2;                //
                                              //
  // -------------------------------------------------------
  // open transaction
  // -------------------------------------------------------
  reason = mqBegin( _ghConn );                // begin transaction
  switch( reason )                            //
  {                                           //
    case MQRC_NONE :                          //
    case MQRC_NO_EXTERNAL_PARTICIPANTS :      // transactions without external 
    {                                         //  resource manager
      sysRc = MQRC_NONE;                      //
      break;                                  //
    }                                         //
    default : goto _door;                     //
  }                                           //
                                              //
  while( msgCnt > 0 )                         //  using signals
  {                                           //
    if( checkSignal() ) break;                // break out of the loop for 
                                              //  any signal
    // -----------------------------------------------------
    // read the message
    // -----------------------------------------------------
    memcpy( &md.MsgId         ,               // flash message id, else only one message in UOW
            MQMI_NONE         ,               // 
            sizeof(md.MsgId) );               // 
                                              //
    reason = mqGet( _ghConn    ,              // connection handle
                    _gohEvQueue,              // pointer to queue handle
                    buffer     ,              // message buffer
                    &msgLng    ,              // buffer length
                    &md        ,              // msg Desriptor
                    gmo        ,              // get message option
                    60000     );              // wait interval in milliseconds
                                              // (makes 1Minute)
    switch( reason )                          //
    {                                         //
      case MQRC_NONE :                        //
      {                                       //
        msgCnt--;                             //
        movedMessages=transSize-msgCnt;       //
        break;                                // ok
      }                                       //
      case MQRC_NO_MSG_AVAILABLE :            // no message available, necessary 
      {                                       //  for catching signals
	sysRc = reason;                       //
        goto _door;                           //
      }                                       //
      case MQRC_BACKED_OUT:                   //
      {                                       //
	sysRc = reason;                       //
	goto _door;                           //
      }                                       //
      case MQRC_TRUNCATED_MSG_FAILED :        // message buffer to small for 
      {                                       //  the physical message, 
        logMQCall( WAR, "MQGET", reason );    //  resize (realloc) the buffer
	bufferSize = msgLng;                  //
        buffer = resizeMqMessageBuffer( buffer, &bufferSize );
                                              //
        // -------------------------------------------------
        // read the message with new buffer size
        // -------------------------------------------------
        sysRc = mqGet( _ghConn     ,          // connection handle
                        _gohEvQueue,          // pointer to queue handle
                        buffer     ,          // message buffer
                        &msgLng    ,          // buffer length
                        &md        ,          // msg Desriptor
                        gmo        ,          // get message option
                        500       );          //  wait interval
                                              //
	if( sysRc != MQRC_NONE )              //
        {                                     //
          goto _door;                         //
        }                                     //
        continue;                             //
      }                                       //
      default :                               // real error (stopping qmgr)
      {                                       //
        sysRc = reason;                       //
        goto _door;                           //
      }                                       //
    }                                         //
                                              //
    // ---------------------------------------------------  
    // write the same message to done queue
    // ---------------------------------------------------  
    pmo.Options=MQPMO_FAIL_IF_QUIESCING +     //
  //            MQPMO_NO_CONTEXT        +     //
                MQPMO_SET_ALL_CONTEXT   +     // for keeping old time / date
                MQPMO_SYNCPOINT         ;     //
                                              //
    reason = mqPut( _ghConn       ,           // connection handle
                    _gohStoreQueue,           // pointer to queue handle
                    &md           ,           // message descriptor
                    &pmo          ,           // Options controlling MQPUT
                    buffer        ,           // message buffer
                    msgLng       );           // message length (buffer length)
                                              //
    switch( reason )                          //
    {                                         //
      case MQRC_NONE :                        // message put to store queue
      {                                       //
        for( i=0; i<TRANSACTION_SIZE; i++ )   // find out the queue manager 
        {                                     //  where event was produced and 
	  if( memcmp( _movedMsgQmgr[i]     ,  //  put it's name to the 
                      md.ReplyToQMgr       ,  //  _moveMsgQmgr array,
                      MQ_Q_MGR_NAME_LENGTH ) == 0)      
          {                                   // ignore queue manager already 
            break ;                           //  in the array (found in the 
          }                                   //  same transaction)
          if( _movedMsgQmgr[i][0]=='\0' )     //
          {                                   //
	    memcpy( _movedMsgQmgr[i]     ,    //
                    md.ReplyToQMgr       ,    //
                    MQ_Q_MGR_NAME_LENGTH);    //
	    break ;                           //
          }                                   //
        }                                     //
        break;                                //
      }                                       //
      case MQRC_BACKED_OUT:                   //
      {                                       //
	sysRc = reason;                       //
	goto _door;                           //
      }                                       //
      default :                               //
      {                                       //
        sysRc = reason;                       //
        goto _door;                           //
      }                                       //
    }                                         //
  }                                           //
                                              //
  // -------------------------------------------------------  
  // commit or rollback transaction
  // -------------------------------------------------------  
  _door :                                     //
                                              //
  if( movedMessages > 0          &&           // messages found and moved
      ( sysRc==MQRC_NONE         ||           // full transaction, 
        sysRc== MQRC_NO_MSG_AVAILABLE ) )     // got all messages, queue empty  
  {                                           //
    reason = mqCommit( _ghConn );             //  commit transaction
    switch( reason )                          //
    {                                         //
      case MQRC_NONE : break;                 //
      case MQRC_NO_EXTERNAL_PARTICIPANTS:     // MQ only, no external 
      {                                       //  participants 
        sysRc = MQRC_NONE;                    // set sysRc to 0,  to avoid 
        break;                                //  evaluating RC in calling
      }                                       //  functions 
      default :                               //
      {                                       //
        sysRc = reason;                       //
	break;                                //
      }                                       //
    }                                         //
    for( i=0; i<TRANSACTION_SIZE; i++ )       // trim blanks out of 
    {                                         // queue manager name
      if( _movedMsgQmgr[i][0]=='\0' )         //
      {                                       //
        break;                                //
      }                                       //
      for( j=0; j< MQ_Q_MGR_NAME_LENGTH; j++ )//
      {                                       //
        if( _movedMsgQmgr[i][j] == '\0' )     //
	{                                     //
          break;                              //
	}                                     //
        if( _movedMsgQmgr[i][j] == ' ' )      //
	{                                     //
          _movedMsgQmgr[i][j] ='\0' ;         //
          break;                              //
	}                                     //
      }                                       //
    }                                         //
  }                                           //
  else                                        // some error occurred
  {                                           //
    memset( _movedMsgQmgr, 0      ,           // rollback 
            MQ_Q_MGR_NAME_LENGTH  *           //
            TRANSACTION_SIZE      *           //
            sizeof(char)         );           //
    reason = mqRollback( _ghConn );           //
    switch( reason )                          //
    {                                         //
      case MQRC_NONE : break ;                //
      default : break;                        //
    }                                         //
  }                                           //
                                              //
  logFuncExit( );
  return sysRc  ;
}
