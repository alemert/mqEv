/******************************************************************************/
/*   M Q   E V E N T                                                          */
/*                                                                            */
/*   description:                                                  */
/*     central mq module handling mq event monitoring            */
/*                                                                            */
/*   functions:                                          */
/*    - initMq                                  */
/*    - browseEvents                          */
/*                                    */
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

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
  MQHCONN _ghConn ;
  MQOD    _godEvQueue = {MQOD_DEFAULT} ;
  MQHOBJ  _gohEvQueue ;

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

/******************************************************************************/
/* init mq connection                              */
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
  // open queue
  // -------------------------------------------------------
  char *evQueue = getStrAttr( "queue" );
  if( evQueue == NULL )
  {
     evQueue = getIniStrValue( getIniNode("mq","qmgr","queue",NULL), "name" ); 
  }
 
  if( evQueue == NULL )
  {
    logger( LSTD_UNKNOWN_CMDLN_ATTR, "queue" ) ;
    logger( LSTD_UNKNOWN_INI_ATTR, "mq - qmgr - queue - name" );
    logger( LSTD_GEN_ERR, __FUNCTION__ );
    sysRc = 1 ; 
    goto _door ; 
  }

  memset( _godEvQueue.ObjectName, (int) ' ', MQ_Q_NAME_LENGTH ) ;
  memcpy( _godEvQueue.ObjectName, evQueue  , MQ_Q_NAME_LENGTH ) ;

  sysRc = mqOpenObject( 
              _ghConn                ,  // qmgr connection handle 
              &_godEvQueue           ,  // event q object descriptor 
              MQOO_INPUT_AS_Q_DEF    |  //   open object for get
              MQOO_BROWSE            |  //   open for browse
              MQOO_FAIL_IF_QUIESCING ,  //   open fails if qmgr is stopping
              &_gohEvQueue          );  // object handle event queue

  switch( sysRc )                       // rc mqopen
  {                                     //
    case MQRC_NONE : break ;            // open ok
    default        : goto _door ;       // open failed
  }                                     //
      
  _door :

  logFuncExit( ) ;

  return sysRc ;
}


/******************************************************************************/
/* browse events            */
/******************************************************************************/
int browseEvents( )
{
  logFuncCall( ) ;

  int sysRc = 0 ;

  MQMD  evMsgDscr = {MQMD_DEFAULT};  // message descriptor (set to default)
  MQGMO getMsgOpt = _gDefGMO;        // get message option set to default
                                     //
  MQHBAG evBag = MQHB_UNUSABLE_HBAG; //
                                     //
  MQLONG  compCode;                  //
  MQLONG  reason   = MQRC_NONE;      //
                                     //
  sysRc = reason;                    //
                                     //
  // -------------------------------------------------------  
  // init mq for get events
  // -------------------------------------------------------  
  getMsgOpt.Options = MQGMO_WAIT              // wait for new messages
                    + MQGMO_FAIL_IF_QUIESCING // fail if quiesching
                    + MQGMO_CONVERT           // convert if necessary
                    + MQGMO_BROWSE_NEXT;      // browse messages
  getMsgOpt.Version = MQGMO_VERSION_2;        // gmo ver 2 for bags
                                              //
  mqCreateBag( MQCBO_USER_BAG,                // create a user bag
               &evBag        ,                // 
               &compCode     ,                //
               &reason      );                //
                                              //
  switch( reason )                            // handle mqCreateBag error
  {                                           //
    case MQRC_NONE :                          //
    {                                         //
      logMQCall(DBG,"mqCreateBag",reason);    //
      break;                                  //
    }                                         //
    default :                                 //
    {                                         //
      logMQCall(ERR,"mqCreateBag",reason);    //
      sysRc = reason;                         //
      goto _door;                             //
    }                                         //
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
    mqGetBag( _ghConn    ,                    // global (qmgr) connect handle
              _gohEvQueue,                    // globale (queue) open handle
              &evMsgDscr ,                    // message descriptor
              &getMsgOpt ,                    // get message options
              evBag      ,                    // event bag
              &compCode  ,                    // compelition code
              &reason   );                    // mq reason
                                              //
    dumpMqStruct( "GMO  ", &getMsgOpt, NULL );//
    dumpMqStruct( "MD   ", &evMsgDscr, NULL );//
                                              //
    switch( reason )                          //
    {                                         //
      case MQRC_NONE :                        //
      {                                       //
        logMQCall( DBG, "mqGetBag", reason ); //
        continue;                             //
      }                                       //
      case MQRC_NO_MSG_AVAILABLE :            //
      {                                       //
        logMQCall( DBG, "mqGetBag", reason ); //
        continue;                             //
      }                                       //
      default :                               //
      {                                       //
        logMQCall( ERR, "mqGetBag", reason ); //
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
  if( evBag != MQHB_UNUSABLE_HBAG )           //
  {              //
    mqDeleteBag( &evBag   ,       //
                 &compCode,       //
                 &reason );      //
                //
    switch( reason )                          // handle mqCreateBag error
    {                                         //
      case MQRC_NONE :                        //
      {                                       //
        logMQCall(DBG,"mqDeleteBag",reason);  //
        break;                                //
      }                                       //
      default :                               //
      {                                       //
        logMQCall(ERR,"mqDeleteBag",reason);  //
        sysRc = reason;                    //
        goto _door;                           //
      }                                       //
    }                                         //
  }

  _door:

  logFuncExit( ) ;
  return sysRc ;
}
