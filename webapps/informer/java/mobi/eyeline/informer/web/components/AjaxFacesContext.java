package mobi.eyeline.informer.web.components;

import org.apache.myfaces.context.FacesContextWrapper;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class AjaxFacesContext extends FacesContextWrapper {

  private final String ajaxComponentId;
  private boolean skipContent=true;

  public AjaxFacesContext(FacesContext facesContext, String ajaxComponentId) {
    super(facesContext);
    this.ajaxComponentId = ajaxComponentId;
  }

  public void setSkipContent(boolean skipContent) {
    this.skipContent = skipContent;
  }

  public ResponseWriter getAjaxResponseWriter() {
    return super.getResponseWriter();
  }

  public ResponseWriter getResponseWriter() {
    if (skipContent)
      return new BulkResponseWriter(super.getResponseWriter());
    else
      return super.getResponseWriter();
  }

  public String getAjaxComponentId() {
    return ajaxComponentId;
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
