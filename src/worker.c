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
/*  xymon worker                                                */
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
/*  console worker                                              */
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
  browseEvents();

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
/*  html worker                        */
/******************************************************************************/
int htmlWorker()
{
  logFuncCall( ) ;
  int sysRc = 0 ;

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
  sysRc = initMq();                      // connect to Queue Manager and open collect and acknowledge queue
  if( sysRc != 0 )                      //
  {                                       //
    goto _door;                            //
  }                                      //
                                    //
  while( 1 )                        //
  {                                    //
    // -----------------------------------------------------
    // browse messages in input queue
    // -----------------------------------------------------
    sysRc = browseEvents();            // browse events 
    if( sysRc != 0 )                           //
    {                                          //
      goto _door;                              //
    }                                          //
                                          //
    // -----------------------------------------------------
    // handle events that can be moved to acknowledge queue automatically
    // -----------------------------------------------------
    sysRc = handleDoneEvents();            // match stop / start events and move them to acknowledge queue
    if( sysRc > 0 )                            //
    {                                          //
      goto _door;                              //
    }                                          //
    else if( sysRc < 0 )                // some events where moved through handleDoneEvents, the collect queue has to be re-read
    {      //
      sysRc = browseEvents();              //
      if( sysRc != 0 )                         //
      {                                        //
        goto _door;                            //
      }                                        //
    }
                                        //
    // -----------------------------------------------------
    // list events on html
    // -----------------------------------------------------
    sysRc = printAllEventTable( wwwDir );      //
    if( sysRc != 0 )              //
    {                          //
      goto _door;              //
    }                          //
                    //
    sleep(1);      //
  }                              //
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
  if( !wwwDir )                        //
  {                                  //
    sysRc = 1;                    //
    goto _door;                    //
  }                              //
                                //
  sysRc = initMq();                       // connect to queue manager 
  if( sysRc != 0 )                        // and open the queues
  {                                       // handle mq errors
    goto _door;                           //
  }                                       //
                                          //
  sysRc = acknowledgeMessages( );         // acknowledge messages through 
  if( sysRc != 0 )                  // moving them from collection to acknowledge queue
  {                              //
    goto _door;                           //
  }                                //
                                          //
  sysRc = browseEvents();      //
  if( sysRc != 0 )                  //
  {                                  //
    goto _door;                    //
  }                            //
                              //
  endMq();                          // ignore reason, end of program
                              //
  sysRc = printAllEventTable( wwwDir );   //
  if( sysRc != 0 )          //
  {                                  //
    goto _door;            //
  }                                  //
  _door :                                 // 
  
  logFuncExit( ) ;
  return sysRc ;
}
