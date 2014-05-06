/******************************************************************************/
/*   M Q   E V E N T S                                                        */
/*                                                                            */
/*    Worker functions:                                        */
/*      - xymonWorker                                                */
/*      - consoleWorker                                          */
/*      - htmlWorker                          */
/*      - ackWorker                    */
/*                                                          */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <unistd.h>

// ---------------------------------------------------------
// MQ
// ---------------------------------------------------------
#include <cmqc.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include "cmdln.h"
#include <inihnd.h>
#include <ctl.h>
#include <msgcat/lgstd.h>
#include <mqbase.h>
#include <worker.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <mqev.h>
#include <dumper.h>
#include <node.h>

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

/******************************************************************************/
/*  xymon worker                                                              */
/******************************************************************************/
int xymonWorker( )
{
  logFuncCall( ) ;

  int sysRc = 0 ;

#if(0)
  _door :
#endif
  logFuncExit( ) ;

  return sysRc ;
}

/******************************************************************************/
/*  console worker                                                            */
/******************************************************************************/
int consoleWorker()
{
  logFuncCall( ) ;

  int sysRc = 0 ;

  sysRc = initMq();
  if( sysRc != 0 )
  {
    goto _door ;
  }

  // -------------------------------------------------------
  // browse messages in input queue
  // -------------------------------------------------------
  browseEvents( _gohStoreQueue, _gohErrQueue );

  handleDoneEvents();

  // -------------------------------------------------------
  // list events
  // -------------------------------------------------------
  printAllEventList();

  _door :

  logFuncExit( ) ;
  return sysRc ;
}


/******************************************************************************/
/*  HTML worker                                                               */
/******************************************************************************/
int htmlWorker()
{
  logFuncCall( ) ;
  int sysRc = 0 ;

  char movedMsgQmgr[TRANSACTION_SIZE][MQ_Q_MGR_NAME_LENGTH+1] ;

  tIniNode *searchIni ;    // data structure for getting searching in ini files
  char     *wwwDir    ;    // directory for raw html data
  char     *evCfgFile ;    // XML file name for the event level configuration

  // -------------------------------------------------------
  // initialize HTML worker
  // -------------------------------------------------------
  searchIni = getIniNode( "system", "html" );  // system.html node from ini
  wwwDir = getIniStrValue( searchIni,"dir" );  // get file & level from node
  if( !wwwDir )                                // abort if html-interface 
  {                                            //  directory does not exist
    sysRc = 1 ;                                //
    goto _door;                                //
  }                                            //
                                               //
  searchIni = getIniNode("system","event");    // system.event node from ini
  evCfgFile = getIniStrValue(searchIni,"ini"); // get ini from node
  if( !evCfgFile )
  {                                            // event config file not
    sysRc = 1 ;                                //  found in ini file
    goto _door;                                //
  }                                            //
  if( loadCfgEvent( evCfgFile ) > 0 )          // load level for the events 
  {                                            //  from XML file
    sysRc = 1 ;                                //
    goto _door;                                //
  }                                            //
                                               //
  sysRc = initMq();                            // connect to Queue Manager 
  if( sysRc != 0 )                             //  and open the collect and
  {                                            //  the acknowledge queue
    goto _door;                                //
  }                                            //
                                               //
  // -------------------------------------------------------
  // flush old events from existing files
  // -------------------------------------------------------
  flushEventFiles( wwwDir ); 

  // -------------------------------------------------------
  // major loop
  // -------------------------------------------------------
  while( 1 )                                   //
  {                                            //
    freeEventTree();                           // get rit of old events
                                               //
    // -----------------------------------------------------
    // browse messages in the store queue
    // -----------------------------------------------------
    sysRc = browseEvents( _gohStoreQueue,      // browse events and move 
                          _gohErrQueue );      //  unknown events to error queue
    if( sysRc != 0 )                           //
    {                                          //
      goto _door;                              //
    }                                          //
                                               //
    // -----------------------------------------------------
    // handle events that can be moved to acknowledge queue automatically
    // -----------------------------------------------------
    sysRc = handleDoneEvents();                // match stop / start events 
    if( sysRc > 0 )                            //  and move them to the 
    {                                          //  acknowledge queue
      goto _door;                              //
    }                                          //
    else if( sysRc < 0 )                       // handleDoneEvents moved some 
    {                                          //  events, therefor the collect 
      sysRc = browseEvents( _gohStoreQueue,    //
                            _gohErrQueue );    //  queue has to be re-read
      if( sysRc != 0 )                         //
      {                                        //
        goto _door;                            //
      }                                        //
    }                                          //
                                               //
    // -----------------------------------------------------
    // list events on html
    // -----------------------------------------------------
    sysRc = printAllEventTable( wwwDir );      // write data to the WWW-interface 
    if( sysRc != 0 )                           //  directory
    {                                          //
      goto _door;                              //
    }                                          //
                                               //
    // -----------------------------------------------------
    // accept messages
    // -----------------------------------------------------
    do
    {                                          // wait for messages on the 
      sysRc = acceptMessages( movedMsgQmgr );  //  collect queue and move them 
      switch( sysRc )                          //  to the store queue
      {                                        //
        case MQRC_NONE :                       // write time stamp flag file for  
        {                                      //  each queue manager that 
          touchEventFlag(wwwDir,movedMsgQmgr); //  produced an event message
	  break;                               //
	}                                      //
        case MQRC_NO_MSG_AVAILABLE :           //
	{                                      //
          usleep(5000);                        // sleep milli seconds for 
	  break;                               //  handling signals
	}                                      //
        default : goto _door;                  //
      }                                        //
    }                                          //
    while( movedMsgQmgr[0][0] == '\0' );       // stay in loop if no 
                                               //  message found
                                               //
    sleep(1);                                  //
  }                                            //
                                               //
  _door :

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*  acknowledge worker                  */
/******************************************************************************/
int ackWorker( )
{
  logFuncCall( ) ;
  int sysRc = 0 ;

  tIniNode *searchIni ;    // data structure for getting searching in ini files
  char     *wwwDir    ;    // directory for raw html data

  searchIni = getIniNode("system", "html"); // system.html node from ini
  wwwDir = getIniStrValue(searchIni,"dir"); // get file & level from node
  if( !wwwDir )                             //
  {                                         //
    sysRc = 1;                              //
    goto _door;                             //
  }                                         //
                                            //
  // -------------------------------------------------------  
  // initialize MQ
  // -------------------------------------------------------  
  sysRc = initMq();                         // connect to queue manager 
  if( sysRc != 0 )                          // and open the queues
  {                                         // handle mq errors
    goto _door;                             //
  }                                         //
                                            //
  // -------------------------------------------------------  
  // browse all events
  // -------------------------------------------------------  
  sysRc = browseEvents( _gohStoreQueue,     // read all events
                        _gohErrQueue );     //
  if( sysRc != 0 )                          //
  {                                         // free the event tree, all queue 
    goto _door;                             // manager are still in the event tree.
  }                                         // 
                                            // this procedure is necessary if 
  freeEventTree();                          // the last event of some queue 
                                            // manager is acknowledged
  // -------------------------------------------------------  
  // acknowledge messages 
  // -------------------------------------------------------  
  sysRc = acknowledgeMessages( );           // acknowledge messages through 
  if( sysRc != 0 )                          //  moving them from store to the 
  {                                         //  acknowledge queue
    goto _door;                             //
  }                                         //
                                            //
  // -------------------------------------------------------  
  // browse rest messages 
  // -------------------------------------------------------  
  sysRc = browseEvents( _gohStoreQueue,     //
                        _gohErrQueue );     //
  if( sysRc != 0 )                          //
  {                                         //
    goto _door;                             //
  }                                         //
                                            //
#if(0)                                      // ignore reason, end of program
  endMq();                                  // endMq is handled by signals
#endif                                      //
                                            //
  // -------------------------------------------------------  
  // data output
  // -------------------------------------------------------  
  sysRc = printAllEventTable( wwwDir );     //
  if( sysRc != 0 )                          //
  {                                         //
    goto _door;                             //
  }                                         //
  _door :                                   // 
  
  logFuncExit( ) ;
  return sysRc ;
}
