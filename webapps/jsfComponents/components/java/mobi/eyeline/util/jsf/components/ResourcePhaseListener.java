package mobi.eyeline.util.jsf.components;

import javax.faces.FacesException;
import javax.faces.context.ExternalContext;
import javax.faces.context.FacesContext;
import javax.faces.event.PhaseEvent;
import javax.faces.event.PhaseId;
import javax.faces.event.PhaseListener;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */

public class
    ResourcePhaseListener implements PhaseListener {
  private static final String	WEBLETS_PHASE_LISTENER_ENTERED	= "net.java.dev.weblets.impl.faces.WebletsPhaseListener.entered";

  public void afterPhase(PhaseEvent event) {
  }

  public void beforePhase(PhaseEvent event) {

    if (event.getPhaseId() != PhaseId.RESTORE_VIEW && event.getPhaseId() != PhaseId.APPLY_REQUEST_VALUES && event.getPhaseId() != PhaseId.RENDER_RESPONSE) {
      // try to execute the phase-listener logic only on apply request values
      // and (if restore-view is not called, e.g. in the portlet-case) on render-response
      return;
    }
    Boolean isReentry = (Boolean) event.getFacesContext().getExternalContext().getRequestMap().get(WEBLETS_PHASE_LISTENER_ENTERED);
    if (isReentry != Boolean.TRUE) {
      event.getFacesContext().getExternalContext().getRequestMap().put(WEBLETS_PHASE_LISTENER_ENTERED, Boolean.TRUE);
      doBeforePhase(event);
    }
  }

  protected void doBeforePhase(PhaseEvent event) {
    FacesContext context = FacesContext.getCurrentInstance();
    ExternalContext external = context.getExternalContext();

    String pathInfo = external.getRequestServletPath();
    if (pathInfo != null && external.getRequestPathInfo() != null)
      pathInfo += external.getRequestPathInfo();
    if (pathInfo == null && event.getPhaseId() == PhaseId.RESTORE_VIEW) {
      // we are in a portlet environment here, since we do not get an external path info
      // we skip this phase and renter after the restore view
      event.getFacesContext().getExternalContext().getRequestMap().put(WEBLETS_PHASE_LISTENER_ENTERED, Boolean.FALSE);
      return;
    }
    // special portlet treatment here
    // we move to later phases here to the apply request values
    // to become portlet compatible, unfortunately
    // we lose a little bit of performance then
    // but the determination of the pathInfo over
    // the view id is more neutral than over
    // the request servlet which is rundered null
    // in some portlet environments
    if (pathInfo == null)
      pathInfo = context.getViewRoot().getViewId();


    if (pathInfo.contains(ResourceUtils.COMPONENTS_PATH)) {
      try{
        ResourceUtils.writeResource(pathInfo, external);
        context.responseComplete();
      } catch (IOException e) {
        throw new FacesException(e);
      }
    }
  }

  public PhaseId getPhaseId() {
    /*
       * we can only trigger after the restore view due to the fact that the view root is only available then
       */
    return PhaseId.ANY_PHASE;
  }

  /**
   * The serialization version.
   */
  private static final long	serialVersionUID	= -8385571916376473831L;
}
