/******************************************************************************/
/*                             M Q   E V E N T S                              */
/*                                                            */
/*  description:                                  */
/*    start program :                                */
/*      from cmd line                          */
/*      triggered from qmgr                      */
/*    read (browse) mq events from some event queue            */
/*    analyse the event                                  */
/*    show event on the console or send it to the xymon            */
/*    rotate monitoring                              */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdio.h>
#include <libgen.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include "cmdln.h"
#include <ctl.h>
#include <msgcat/lgstd.h>
#include <inihnd.h>

#include <worker.h>

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
int initPrg( int argc, const char* argv[] ) ;

/******************************************************************************/
/*                                                                            */
/*                                  M A I N                                   */
/*                                                                            */
/******************************************************************************/
#ifndef __TDD__

int main(int argc, const char* argv[] )
{
  int sysRc ;

  // -------------------------------------------------------
  // init program 
  // -------------------------------------------------------
  sysRc = initPrg( argc, argv );  // get command line, setup logging
  if( sysRc != 0 ) goto _door;   

  // -------------------------------------------------------
  // main work
  // -------------------------------------------------------
  if( !getFlagAttr( "xymon" ) )
  {
    sysRc = xymonWorker() ;
    goto _door;
  }

  if( !getFlagAttr( "console" ) )
  {
    sysRc = consoleWorker() ;
    goto _door;
  }

  _door :

  return sysRc ;
}

#endif

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*  init program                                                */
/*    - analyse cmdln attributes                                */
/*    - setup logging                                    */
/******************************************************************************/
int initPrg( int argc, const char* argv[] )
{
  int sysRc ;

  // -------------------------------------------------------
  // handle command line
  // -------------------------------------------------------
  sysRc = handleCmdLn( argc, argv ) ;
  if( sysRc != 0 ) goto _door ;

  // -------------------------------------------------------
  // handle ini file
  // -------------------------------------------------------
  const char *ini = getStrAttr( "ini" ) ;
  if( ini )
  {
    sysRc = iniHandler( ini ) ;
    if( sysRc != 0 ) goto _door ;
  }

  // -------------------------------------------------------
  // set logging
  // -------------------------------------------------------
  const char *logName = getStrAttr( "log" ) ;
#if(1)
  tIniNode *searchIni ;
  searchIni = setIniSingleSearchNode( NULL     , "system", NULL, NULL, 0 );
  searchIni = setIniSingleSearchNode( searchIni, "log"   , NULL, NULL, 0 );
  if( logName == NULL ) 
  {
    tIniNode *found = existsIniNode( A_MAIN, NULL, searchIni ) ;
    printf("\n");
  }
#endif
  if( logName == NULL ) 
  {
    sysRc = initLogging( "var/log/mqev.log", INF ) ;
  }
  else
  {
    sysRc = initLogging( logName, INF ) ;
  }
  if( sysRc != 0 ) goto _door ;

  logger( LSTD_PRG_START, basename( (char*) argv[0] ) ) ;
  
  _door :
  return sysRc ;
}


