package ru.sibinco.smppgw.backend.routing;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.SmeManager;

import java.io.File;
import java.util.Map;


/**
 * Created by igork
 * Date: 19.04.2004
 * Time: 17:41:55
 */
public class GwRoutingManager extends RouteSubjectManager
{
  private Logger logger = Logger.getLogger(this.getClass());

  public GwRoutingManager(File smscConfFolder, SmeManager smeManager)
      throws SibincoException
  {
    super(smscConfFolder, smeManager);
    logger.debug("Routes loaded");
  }

  protected Subject createSubject(Element subjElem, SmeManager smeManager) throws SibincoException
  {
    return new Subject(subjElem, smeManager);
  }

  protected Route createRoute(Element routeElem, Map subjects, SmeManager smeManager) throws SibincoException
  {
    return new GwRoute(routeElem, subjects, smeManager, null); //todo: provider
  }
}
