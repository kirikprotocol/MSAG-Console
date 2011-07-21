package mobi.eyeline.util.jsf.components;


import javax.el.ELContext;
import javax.faces.application.Application;
import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.component.UIViewRoot;
import javax.faces.context.ExternalContext;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseStream;
import javax.faces.context.ResponseWriter;
import javax.faces.event.ActionListener;
import javax.faces.render.RenderKit;
import java.io.IOException;
import java.io.Writer;
import java.util.Iterator;

/**
 * @author Artem Snopkov
 */
public class AjaxFacesContext extends FacesContext {

  private final String ajaxComponentId;
  private final FacesContext wrapped;

  public AjaxFacesContext(FacesContext facesContext, String ajaxComponentId) {
    super();
    this.ajaxComponentId = ajaxComponentId;
    this.wrapped = facesContext;
  }

  public void setSkipContent(boolean skipContent) {
    ((AjaxWriter)getResponseWriter()).setSkipContent(skipContent);
  }

  public String getAjaxComponentId() {
    return ajaxComponentId;
  }

  @Override
  public Application getApplication() {
    return wrapped.getApplication();
  }

  @Override
  public Iterator<String> getClientIdsWithMessages() {
    return wrapped.getClientIdsWithMessages();
  }

  @Override
  public ELContext getELContext() {
    return wrapped.getELContext();
  }

  @Override
  public ExternalContext getExternalContext() {
    return wrapped.getExternalContext();
  }

  @Override
  public FacesMessage.Severity getMaximumSeverity() {
    return wrapped.getMaximumSeverity();
  }

  @Override
  public Iterator<FacesMessage> getMessages() {
    return wrapped.getMessages();
  }

  @Override
  public Iterator<FacesMessage> getMessages(String s) {
    return wrapped.getMessages(s);
  }

  @Override
  public RenderKit getRenderKit() {
    return wrapped.getRenderKit();
  }

  @Override
  public boolean getRenderResponse() {
    return wrapped.getRenderResponse();
  }

  @Override
  public boolean getResponseComplete() {
    return wrapped.getResponseComplete();
  }

  @Override
  public ResponseStream getResponseStream() {
    return wrapped.getResponseStream();
  }

  @Override
  public void setResponseStream(ResponseStream responseStream) {
    wrapped.setResponseStream(responseStream);
  }

  @Override
  public ResponseWriter getResponseWriter() {
    return wrapped.getResponseWriter();
  }

  @Override
  public void setResponseWriter(ResponseWriter responseWriter) {
    wrapped.setResponseWriter(responseWriter);
  }

  @Override
  public UIViewRoot getViewRoot() {
    return wrapped.getViewRoot();
  }

  @Override
  public void setViewRoot(UIViewRoot uiViewRoot) {
    wrapped.setViewRoot(uiViewRoot);
  }

  @Override
  public void addMessage(String s, FacesMessage facesMessage) {
    wrapped.addMessage(s, facesMessage);
  }

  @Override
  public void release() {
    wrapped.release();
  }

  @Override
  public void renderResponse() {
    wrapped.renderResponse();
  }

  @Override
  public void responseComplete() {
    wrapped.responseComplete();
  }

}
