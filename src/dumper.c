/******************************************************************************/
/*                     M Q   E V E N T   -   D U M P E R                      */
/*                                                */
/*    functions:                          */
/*      - printAllEventList                  */
/*      - printQmgrEventList              */
/*      - printEventList              */
/*                          */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <stdio.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
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
void printQmgrEventList( tQmgrNode* qmgrNode );
void printEventList( tEvent *eventList );
void printEvent( tEvent *event );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* print all events                                          */
/******************************************************************************/
void printAllEventList()
{
  tQmgrNode* qmgrEventNode = _gEventList ;

  while( qmgrEventNode ) 
  {
    printQmgrEventList( qmgrEventNode );
    qmgrEventNode = qmgrEventNode->next;
  }
}

/******************************************************************************/
/*  print qmgr events                              */
/******************************************************************************/
void printQmgrEventList( tQmgrNode* qmgrNode )
{
  printf("Queue Manager: %s\n", qmgrNode->qmgr );

  printf("Queue Manager Start / Stop: \n" );
  printEventList( qmgrNode->qmgrEvent );
}

/******************************************************************************/
/*  print event list                                */
/******************************************************************************/
void printEventList( tEvent *eventList )
{
  tEvent *event = eventList ;

  while( event )
  {
    printEvent( event->next );
    event = event->next ;
  }
}

/******************************************************************************/
/*  printEvent                                    */
/******************************************************************************/
void printEvent( tEvent *event )
{
    t
}