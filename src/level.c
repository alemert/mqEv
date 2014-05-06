/******************************************************************************/
/*                              M Q   E V E N T                               */
/*                                                                            */
/*  description:                                                              */
/*    module for level handling;                                              */
/*    for each selector should be an error level                 */
/*                                                                            */
/*    functions:                                                              */
/*      - getLevel                                                            */
/*      - getValueLevel                                                       */
/*      - evalEventLevel                                                      */
/*      - getItemLevel                                        */
/*      - loadCfgEvent                                */
/*      - addCfgEvent                          */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*   I N C L U D E S                                                          */
/******************************************************************************/

// ---------------------------------------------------------
// system
// ---------------------------------------------------------
#include <string.h>

// ---------------------------------------------------------
// XML
// ---------------------------------------------------------
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

// ---------------------------------------------------------
// mq
// ---------------------------------------------------------
#include <cmqc.h>
#include <cmqcfc.h>
#include <cmqbc.h>

// ---------------------------------------------------------
// own 
// ---------------------------------------------------------
#include <ctl.h>
#include <msgcat/lgmqe.h>
#include <inihnd.h>
#include <mqtype.h>

// ---------------------------------------------------------
// local
// ---------------------------------------------------------
#include <node.h>
#include <level.h>

/******************************************************************************/
/*   G L O B A L S                                                            */
/******************************************************************************/
tCfgSelector *_gCfgEvent = NULL ;

/******************************************************************************/
/*   D E F I N E S                                                            */
/******************************************************************************/

/******************************************************************************/
/*   M A C R O S                                                              */
/******************************************************************************/

/******************************************************************************/
/*   P R O T O T Y P E S                                                      */
/******************************************************************************/
tCfgSelector* addCfgEvent( MQLONG selector );

/******************************************************************************/
/*                                                                            */
/*   F U N C T I O N S                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*  get level of the selector                                                 */
/******************************************************************************/
tEvLevel getSelectorLevel( MQLONG selector )
{
  int level = MQEV_LEV_NA ;

  switch( selector )
  {
    // -----------------------------------------------------
    // system selectors to be ignored
    // -----------------------------------------------------
    case MQIASY_CODED_CHAR_SET_ID:     // -1, 
    case MQIASY_TYPE             :     // -2, MQCFT_EVENT, 
                                       //     other values should be an error
    case MQIASY_MSG_SEQ_NUMBER   :     // -4, ??? ???? ????
    case MQIASY_CONTROL          :     // -5, PCF Control Option
    case MQIASY_COMP_CODE        :     // -6, Completion code
    case MQIASY_BAG_OPTIONS      :     // -8, read only
    case MQIASY_VERSION          :     // -9,
    {                                  //
      level = MQEV_LEV_IGN;            //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // selectors depending on the value in the item
    // -----------------------------------------------------
    case MQIASY_REASON          :      //   -7, reason code, 
    case MQIACF_REASON_QUALIFIER:      // 1020, reason qualifier
    case MQIACF_ERROR_IDENTIFIER:      // 1013, error identifier,(channel 
    {                                  //       stopped by user is not an error)
      level = MQEV_LEV_EVAL;           //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // informational selectors, general items
    // -----------------------------------------------------
    case MQIASY_COMMAND:               //   -3, event type
    {                                  //
      level = MQEV_LEV_INF;            //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // informational selectors, queue manager event
    // -----------------------------------------------------
    case MQIA_APPL_TYPE   :            // application type
    case MQCACF_APPL_NAME :            // application name
    case MQCA_Q_NAME      :            // queue name
    {                                  //
      level = MQEV_LEV_INF;            //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // informational selectors, channel event
    // -----------------------------------------------------
    case MQCACH_CHANNEL_NAME        :  // 3501, channel name
    case MQCACH_XMIT_Q_NAME         :  // 3505, transmission queue
    case MQCACH_CONNECTION_NAME     :  // 3506, conn name
    case MQIACF_AUX_ERROR_DATA_INT_1:  // 1070, help error information
    case MQIACF_AUX_ERROR_DATA_INT_2:  // 1071, help error information
    case MQCACF_AUX_ERROR_DATA_STR_1:  // 3026, channel name
    case MQCACF_AUX_ERROR_DATA_STR_2:  // 3027, help error information
    case MQCACF_AUX_ERROR_DATA_STR_3:  // 3028, help error information
    {                                  //
      level = MQEV_LEV_INF;            //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // error selectors, queue manager event
    // -----------------------------------------------------
    case MQCACF_USER_IDENTIFIER  :     // not authorized (connect)
    case MQIACF_OPEN_OPTIONS     :     // not authorized (open)
    case MQCA_BASE_OBJECT_NAME   :     // dlq reason
    case MQIA_BASE_TYPE          :     // dlq reason (combined with above)
    {                                  //
      level = MQEV_LEV_ERR;            //
      break;                           //
    }                                  //
                                       //
    // -----------------------------------------------------
    // else, no new level
    // -----------------------------------------------------
    default : break;                   //
  }                                    //

  return level;
}

/******************************************************************************/
/*  get level to value                                                        */
/******************************************************************************/
tEvLevel getValueLevel( MQLONG value )
{
  tEvLevel level;

  switch( value )
  {
    case MQRC_Q_MGR_ACTIVE     :   // start queue manager
    {
      level = MQEV_LEV_INF;
      break;
    }
    case MQRC_UNKNOWN_OBJECT_NAME: // unknown object name
    case MQRC_Q_MGR_NOT_ACTIVE   : // stop queue manager
    case MQRC_NOT_AUTHORIZED     : // 2035: general not authorized
    case MQRQ_CONN_NOT_AUTHORIZED: //    1: connection not authorized
    {
      level = MQEV_LEV_ERR;
      break;
    }
    default :
    {
      logger( LEVN_MISSING_CODE_FOR_SELECTOR, (int) value,
                                              __FILE__, __LINE__ );
      level = MQEV_LEV_NA;
      break;
    }
  }
  return level ;
}

/******************************************************************************/
/*  evaluate event level                                                      */
/*                                                                            */
/*  description:                                                              */
/*    - go through all items (belonging to this event) and find out the       */
/*        highest item level.                                                 */
/*    - the highest item level is an event level                              */
/*                                                                            */
/*      MQEV_LEV_EVAL = 0,   evaluate the level depending on item value       */
/*      MQEV_LEV_IGN  = 1,   ignore( do not show the message)                 */
/*      MQEV_LEV_INF  = 2,   information ( green )                            */
/*      MQEV_LEV_WAR  = 3,   warning  ( yellow )                              */
/*      MQEV_LEV_ERR  = 4,   error    ( red )                                 */
/*      MQEV_LEV_NA   = 5    level not available move message to an error     */
/*                                                                            */
/******************************************************************************/
tEvLevel evalEventLevel( tEvent *_event )
{
  tEvLevel level = MQEV_LEV_EVAL ;
  tMqiItem *item ;

  if( !_event )
  {
    goto _door ;
  }

  item = _event->item ;

  while( item )
  {
    if( item->level > level ) level = item->level ;
    item=item->next ; 
  }

  _event->level = level ;

  _door :

  return level ;
}

/******************************************************************************/
/* get Selector Level          */
/******************************************************************************/
#if(0)
tEvLevel getItemLevel( tMqiItem *mqiItem )
{

  if( _gCfgEvent == NULL )
  {
    tIniNode *searchIni ;                          //
    char* evCfgFile ;

    searchIni = getIniNode("system","event");      // system.event node from ini
    evCfgFile = getIniStrValue( searchIni,"ini" );// get ini from node
    loadCfgEvent( evCfgFile );
  }
}
#endif

/******************************************************************************/
/*  load config event (xml file)                  */
/******************************************************************************/
int loadCfgEvent( const char *evCfgFile )
{
  int sysRc = 0 ;

  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx = NULL ;
  xmlXPathObjectPtr  xpathSelectorObj = NULL ;

  xmlNodeSetPtr selectorNodeSet  = NULL ;
  xmlNodePtr selectorNode  = NULL ;
  int selectorNodeSize ;

  MQLONG selector ;

  int i;

  xmlInitParser();        // initialize XML parser is not reentrant;
                          //   therefor signal handler should be adjusted
                          // should be called only once (must in multi-threads)
                          //
  doc = xmlParseFile( evCfgFile );              // load XML file
  if( doc == NULL )                             //
  {                                             //
    logger(LSTD_XML_FILE_ERR,evCfgFile);        //
    sysRc = 1;                                  //
    goto _door ;                                //
  }                                             //
                                                //
  xpathCtx = xmlXPathNewContext( doc );         // create xmlContext
  if( xpathCtx == NULL )                        //
  {                                             //
    logger( LSTD_XML_CTX_ERR, evCfgFile );      //
    sysRc = 2 ;                                 //
    goto _door ;                                //
  }                                             //
                                                //
  xpathSelectorObj = xmlXPathEvalExpression(    // search for /event/selector
                       (const xmlChar*)  "/event//selector", 
                       xpathCtx             );  //
  if( !xpathSelectorObj )                       // /event//selector not found
  {                                             //
    logger( LSTD_XML_XPATH_ERR, "//selector" ); //
    sysRc = 2 ;                                 //
    goto _door ;                                //
  }                                             //
                                                //
  // -------------------------------------------------------
  // go through all selectors
  // -------------------------------------------------------
  selectorNodeSet=xpathSelectorObj->nodesetval; //
  selectorNodeSize = (selectorNodeSet) ? selectorNodeSet->nodeNr : 0;
  for(i=0;i<selectorNodeSize;i++)      //
  {                                             //
    selectorNode = selectorNodeSet->nodeTab[i]; //
    if( selectorNode->type!=XML_ELEMENT_NODE )  //
      continue ;                                //
    if( strcmp((char*)selectorNode->name,"selector")!=0 ) 
      continue ;                                //
                                                //
    if( selectorNode->properties->type!=XML_ATTRIBUTE_NODE ) 
      continue ;                              //
    if( selectorNode->properties->children->type != XML_TEXT_NODE ) 
      continue ;                          //
    if( strcmp( (char*) selectorNode->properties->name,"id")!=0 ) 
      continue ;                          //
                                               //
    selector = str2mqSelector(
	         (char*) selectorNode->properties->children->content );
    if( selector == MQ_UNKNOWN_SELECTOR )       //
    {                                    //
      logger( LEVN_XML_UNKNWOWN_SELECTOR,       //
              selectorNode->properties->children->content, 
              evCfgFile );              //
      sysRc = 3 ;                      //
      goto _door;            //
    }                        //
    printf("%s\n", selectorNode->properties->children->content );

    addCfgEvent( selector ) ;
    //  hier geht es weiter mit analyse von einzelnen selectoren auf level ebene
  }
  _door:
    if( doc     != NULL ) xmlFreeDoc(doc);
    if(xpathCtx != NULL ) xmlXPathFreeContext( xpathCtx ); 

    return sysRc ;
}


/******************************************************************************/
/*  add event to configuration tree            */
/*                                              */
/*  description:                                */
/*    add event configuration node to the sorted list               */
/******************************************************************************/
tCfgSelector* addCfgEvent( MQLONG selector )
{
  tCfgSelector *cfgEvNode = NULL ;
  tCfgSelector *myEvNode = NULL ;

  // -------------------------------------------------------
  // allocate new event configuration node
  // -------------------------------------------------------
  cfgEvNode = (tCfgSelector*) malloc( sizeof(tCfgSelector) );
  if( !cfgEvNode )
  {
    logger( LSTD_MEM_ALLOC_ERROR );
    goto _door;
  }

  // -------------------------------------------------------
  // initialize node
  // -------------------------------------------------------
  cfgEvNode->selector = selector;
  cfgEvNode->level    = MQEV_LEV_EVAL ;
  cfgEvNode->reason   = NULL ;
  cfgEvNode->next     = NULL ;

  // -------------------------------------------------------
  // add node to the sorted list of nodes
  // -------------------------------------------------------
  if( !_gCfgEvent )                            // solve NULL pointer exception
  {                                            // in case of empty list
    _gCfgEvent = cfgEvNode;                    //
  }                                            //
  else if ( _gCfgEvent->selector > selector )  // if new node selector is
  {                                            //  smaller then the 
    cfgEvNode->next = _gCfgEvent;              //  (old) first one
    _gCfgEvent = cfgEvNode;                    //
  }                                            //
  else                                         // search for the place 
  {                                            //  where new node should be
    myEvNode = _gCfgEvent;                     // inserted to
    while( 1 )                                 //
    {                                          //
      if( !myEvNode->next )                    // the new node is the largest,
      {                                        // put it to the end of the list
        myEvNode->next = cfgEvNode;            //
        break;                                 //
      }                                        //
      if( myEvNode->next->selector > selector) // put new node in front of
      {                                        //  the first node larger then
        cfgEvNode->next = myEvNode->next ;     //  the new one
        myEvNode->next = cfgEvNode;            //
        break;                                 //
      }                                        //
      myEvNode = myEvNode->next;               //
    }                                          //
  }                                            //
  

  _door:
    return cfgEvNode ;
}