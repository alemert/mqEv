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
  browseEvents( _gohStoreQueue );

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

  int movedMessages = 0;
  char movedMsgQmgr[TRANSACTION_SIZE][MQ_Q_MGR_NAME_LENGTH+1] ;

  tIniNode *searchIni ;    // data structure for getting searching in ini files
  char     *wwwDir    ;    // directory for raw html data

  searchIni = getIniNode( "system", "html" );  // system.html node from ini
  wwwDir = getIniStrValue( searchIni,"dir" );  // get file & level from node
  if( !wwwDir )                                // abort if html-interface 
  {                                            //  directory does not exist
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
#if(0)
  // -------------------------------------------------------
  // browse messages in the store queue
  // -------------------------------------------------------
  sysRc = browseEvents( _gohStoreQueue );      // browse events 
  if( sysRc != 0 )                             //
  {                                            //
    goto _door;                                //
  }                                            //
                                               //
  // -------------------------------------------------------
  // list events on html
  // -------------------------------------------------------
  sysRc = printAllEventTable( wwwDir );        // write data to the WWW-interface 
  if( sysRc != 0 )                             //  directory
  {                                            //
    goto _door;                                //
  }                                            //
                                               //
#endif
  // -------------------------------------------------------
  // major loop
  // -------------------------------------------------------
  while( 1 )                                   //
  {                                            //
    // -----------------------------------------------------
    // browse messages in the store queue
    // -----------------------------------------------------
    sysRc = browseEvents( _gohStoreQueue );    // browse events 
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
      sysRc = browseEvents( _gohStoreQueue );  //  queue has to be re-read
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
    {                                          //
      sysRc = acceptMessages( movedMsgQmgr );  // wait for messages on the 
      switch( sysRc )                          //  collect queue and move them 
      {                                        //  to the store queue
        case MQRC_NONE :                       //
        case MQRC_NO_MSG_AVAILABLE :           //
	{                                      //
          usleep(500);                         // sleep milli seconds for 
	  break;                               //  handling signals
	}                                      //
        default : goto _door;                  //
      }                                        //
    }                                          //
    while( movedMessages == 0 );               //
                                               //
    sleep(1);                                  //
  }                                            //
                                               //
  _door :

  logFuncExit( ) ;
  return sysRc ;
}

/******************************************************************************/
/*  acknowledge worker      */
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
  sysRc = initMq();                       // connect to queue manager 
  if( sysRc != 0 )                        // and open the queues
  {                                       // handle mq errors
    goto _door;                           //
  }                                       //
                                          //
  // -------------------------------------------------------  
  // acknowledge messages 
  // -------------------------------------------------------  
  sysRc = acknowledgeMessages( );         // acknowledge messages through 
  if( sysRc != 0 )                        //  moving them from store to the 
  {                                       //  acknowledge queue
    goto _door;                           //
  }                                       //
                                          //
  // -------------------------------------------------------  
  // browse rest messages 
  // -------------------------------------------------------  
  sysRc = browseEvents( _gohStoreQueue ); //
  if( sysRc != 0 )                        //
  {                                       //
    goto _door;                           //
  }                                       //
                                          //
#if(0)                                    //
  endMq();                                // ignore reason, end of program
#endif                                    //
                                          //
  // -------------------------------------------------------  
  // data output
  // -------------------------------------------------------  
  sysRc = printAllEventTable( wwwDir );   //
  if( sysRc != 0 )                        //
  {                                       //
    goto _door;                           //
  }                                       //
  _door :                                 // 
  
  logFuncExit( ) ;
  return sysRc ;
}
