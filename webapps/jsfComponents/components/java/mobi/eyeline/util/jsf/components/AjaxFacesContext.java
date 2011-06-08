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
import javax.faces.render.RenderKit;
import java.io.IOException;
import java.io.Writer;
import java.util.Iterator;

/**
 * @author Artem Snopkov
 */
public class AjaxFacesContext extends FacesContext {

  private final String ajaxComponentId;
  private boolean skipContent = true;

  private FacesContext impl;

  public AjaxFacesContext(FacesContext facesContext, String ajaxComponentId) {
    impl = facesContext;
    this.ajaxComponentId = ajaxComponentId;
  }

  public void setSkipContent(boolean skipContent) {
    this.skipContent = skipContent;
  }

  public ResponseWriter getAjaxResponseWriter() {
    return impl.getResponseWriter();
  }

  public ResponseWriter getResponseWriter() {
    if (skipContent)
      return new BulkResponseWriter(impl.getResponseWriter());
    else
      return impl.getResponseWriter();
  }

  public String getAjaxComponentId() {
    return ajaxComponentId;
  }

  @Override
  public ELContext getELContext() {
    return impl.getELContext();
  }

  @Override
  public Application getApplication() {
    return impl.getApplication();
  }

  @Override
  public Iterator<String> getClientIdsWithMessages() {
    return impl.getClientIdsWithMessages();
  }

  @Override
  public ExternalContext getExternalContext() {
    return impl.getExternalContext();
  }

  @Override
  public FacesMessage.Severity getMaximumSeverity() {
    return impl.getMaximumSeverity();
  }

  @Override
  public Iterator<FacesMessage> getMessages() {
    return impl.getMessages();
  }

  @Override
  public Iterator<FacesMessage> getMessages(String s) {
    return impl.getMessages(s);
  }

  @Override
  public RenderKit getRenderKit() {
    return impl.getRenderKit();
  }

  @Override
  public boolean getRenderResponse() {
    return impl.getRenderResponse();
  }

  @Override
  public boolean getResponseComplete() {
    return impl.getResponseComplete();
  }

  @Override
  public ResponseStream getResponseStream() {
    return impl.getResponseStream();
  }

  @Override
  public void setResponseStream(ResponseStream responseStream) {
    impl.setResponseStream(responseStream);
  }

  @Override
  public void setResponseWriter(ResponseWriter writer) {
    impl.setResponseWriter(writer);
  }

  @Override
  public UIViewRoot getViewRoot() {
    return impl.getViewRoot();
  }

  @Override
  public void setViewRoot(UIViewRoot uiViewRoot) {
    impl.setViewRoot(uiViewRoot);
  }

  @Override
  public void addMessage(String s, FacesMessage facesMessage) {
    impl.addMessage(s, facesMessage);
  }

  @Override
  public void release() {
    impl.release();
  }

  @Override
  public void renderResponse() {
    impl.renderResponse();
  }

  @Override
  public void responseComplete() {
    impl.responseComplete();
  }

  private static class BulkResponseWriter extends ResponseWriter {

    private ResponseWriter w;

    private BulkResponseWriter(ResponseWriter w) {
      this.w = w;
    }

    @Override
    public String getContentType() {
      return w.getContentType();
    }

    @Override
    public String getCharacterEncoding() {
      return w.getCharacterEncoding();
    }

    @Override
    public void write(char[] cbuf, int off, int len) throws IOException {
    }

    @Override
    public void flush() throws IOException {
    }

    @Override
    public void close() throws IOException {
    }

    @Override
    public void startDocument() throws IOException {
    }

    @Override
    public void endDocument() throws IOException {
    }

    @Override
    public void startElement(String s, UIComponent uiComponent) throws IOException {
    }

    @Override
    public void endElement(String s) throws IOException {
    }

    @Override
    public void writeAttribute(String s, Object o, String s1) throws IOException {
    }

    @Override
    public void writeURIAttribute(String s, Object o, String s1) throws IOException {
    }

    @Override
    public void writeComment(Object o) throws IOException {
    }

    @Override
    public void writeText(Object o, String s) throws IOException {
    }

    @Override
    public void writeText(char[] chars, int i, int i1) throws IOException {
    }

    @Override
    public ResponseWriter cloneWithWriter(Writer writer) {
      return this;
    }
  }
}
