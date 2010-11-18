package mobi.eyeline.informer.web.controllers.restriction;

import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * @author Artem Snopkov
 */
public class RestrictionController extends InformerController {

  public boolean isRestrictionDaemonStarted() {
    return getConfig().isResctictionDaemonStarted();
  }
}
