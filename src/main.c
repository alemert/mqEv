/******************************************************************************/
/*                             M Q   E V E N T S                              */
/*                                                                            */
/*  description:                                                              */
/*                                                        */
/*    start program :                                                         */
/*      from cmd line                                                         */
/*      triggered from qmgr (not applied yet)                                 */
/*                                                  */
/*    work flow:                                        */
/*     - read events from ADMIN.COLLECT.QUEUE and                 */
/*     - put them on the  ADMIN.STORE.QUEUE                        */
/*     - double (i.g. stop/start) events are automatically forwarded from     */
/*       ADMIN.STORE.QUEUE to ADMIN.CONFIRM.QUEUE                      */
/*     - single events (f.e. dlq ) have to be acknowledged by web interface.  */
/*       web interface will move them from ADMIN.STORE.QUEUE             */
/*       to ADMIN.COLLECT.QUEUE                                  */ 
/*     - each event on the ADMIN.STORE.QUEUE and ADMIN.COLLECT.QUEUE will be  */
/*       analysed and written to web interface file (or shown on console      */
/*                                                                            */
/*    functions:                                                    */
/*      - main                                                              */
/*      - initPrg                                                  */
/*      - background                                                          */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include "cmdln.h"
#include <ctl.h>
#include <msgcat/lgstd.h>
#include <inihnd.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <worker.h>
#include <process.h>

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
int background();

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

  if(!getFlagAttr("html"))
  {
    sysRc = htmlWorker() ;
    goto _door;
  }

  if( getStrArrayAttr( "ack" ) )
  {
    sysRc = ackWorker() ;
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
/*  init program                                                      */
/*    - analyse cmdln attributes                                    */
/*    - setup logging                                        */
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
  char *logName  = getStrAttr( "log" );            // get logfile name from 
  char *logLevelBuff;                              //  command line
  int  logLevel = INF;                             // default log level
                                                   //
  if( logName == NULL )                            // if log not set by cmdln
  {                                                // try to set it from ini
    tIniNode *searchIni ;                          //
    searchIni = getIniNode("system","log");        // system.log node from ini
    logName   = getIniStrValue( searchIni,"file" );// get file & level from node
    logLevelBuff = getIniStrValue( searchIni,"level" ); 
    if( logLevelBuff )                             // if log level set by ini
    {                                              // change it from default 
      logLevel = logStr2lev( logLevelBuff );       //  to ini-set value
    }                                              //
  }                                                //
                                                   //
  if( logName == NULL )                            // if log file name not set
  {                                                //  by cmdln or ini
    sysRc=initLogging("var/log/mqev.log",logLevel);//  set it to "some" default
  }                                                //
  else                                             //
  {                                                //
    sysRc = initLogging( logName, logLevel ) ;     // set log file name to
  }                                                // cmdln-cfg or ini-cfg
  if( sysRc != 0 )       //
  {                //
    fprintf(stderr,"logging not possible, abort\n");//
    goto _door;                     //
  }                  //
                                                   //
  logger(LSTD_PRG_START,basename((char*) argv[0]));//
                                                   //
  setSignals( basename((char*) argv[0]) );         //
                                                   //
  // -------------------------------------------------------
  // send process to background
  // -------------------------------------------------------
  tIniNode *searchIni ;                            //
  searchIni = getIniNode( "system" );              // system node from ini
  char *daemonFlag = getIniStrValue( searchIni,"daemon" );// get daemon flag
  if( !daemonFlag ) goto _door;
  if( strcmp( daemonFlag, "yes") == 0 )
  {
    sysRc = background();
    if( sysRc != 0 )
    {
      goto _door;
    }
  }


  _door :
  return sysRc ;
}

/******************************************************************************/
/*  send process to background                          */
/******************************************************************************/
int background()
{

  logger( LSTD_FORK_PROCESS );
  pid_t pid = fork();

  if( pid == 0 ) 
  {
    logger( LSTD_FORK_CHILD );
    goto _door ;
  }

  if( pid > 0 ) 
  {
    logger( LSTD_FORK_PARENT );
    exit(0);
  }

  logger( LSTD_FORK_FAILED );
  exit(1);

  _door:
  return pid ;
}
