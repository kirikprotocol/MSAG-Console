package mobi.eyeline.informer.web;

import org.apache.log4j.Logger;

import javax.faces.context.FacesContext;
import javax.servlet.http.HttpServletResponse;

/**
 * @author Aleksandr Khalitov
 */

public class ErrorHandler {

  private static final Logger logger = Logger.getLogger(ErrorHandler.class);

  public void handleException(FacesContext context, Exception ex) throws Throwable {
    if(ex.getClass().getName().equals("javax.faces.application.ViewExpiredException")) {
      HttpServletResponse response = (HttpServletResponse)context.getExternalContext().getResponse();
      response.sendRedirect(context.getExternalContext().getRequestContextPath()+"/index.html");
    } else {
      logger.error(ex, ex);
      ex.printStackTrace();
      HttpServletResponse response = (HttpServletResponse)context.getExternalContext().getResponse();
      response.sendRedirect(context.getExternalContext().getRequestContextPath()+"/redirect?url=/error500.faces");
    }
  }
}


