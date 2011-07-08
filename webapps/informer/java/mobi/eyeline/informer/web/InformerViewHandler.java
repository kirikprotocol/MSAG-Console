package mobi.eyeline.informer.web;

import com.sun.facelets.FaceletViewHandler;
import mobi.eyeline.informer.web.components.AjaxFacesContext;
import mobi.eyeline.informer.web.components.AjaxWriter;

import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Locale;
import java.util.Map;

/**
 * author: alkhal
 */
public class InformerViewHandler extends FaceletViewHandler {

  public InformerViewHandler(javax.faces.application.ViewHandler parent) {
    super(parent);
  }

  private static final String VIEW_CACHE_PARAM = "org.apache.myfaces.trinidadinternal.application.VIEW_CACHE";

  private static void replaceState(FacesContext context, String oldKey, String newKey) {
    Map<String, Object> sessionMap = context.getExternalContext().getSessionMap();

    Object newState = sessionMap.remove(VIEW_CACHE_PARAM + '.' + newKey);
    sessionMap.put(VIEW_CACHE_PARAM + '.' + oldKey, newState);
  }

  @Override
  protected ResponseWriter createResponseWriter(FacesContext context) throws IOException, FacesException {
    String p = context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
    if (p != null) {
      return new AjaxWriter(super.createResponseWriter(context));
    }else {
      return super.createResponseWriter(context);
    }

  }

  public void renderView(javax.faces.context.FacesContext context, javax.faces.component.UIViewRoot viewToRender) throws java.io.IOException, javax.faces.FacesException {

    String p = context.getExternalContext().getRequestParameterMap().get("eyelineComponentUpdate");
    if (p != null) {
      super.renderView(new AjaxFacesContext(context, p), viewToRender);

      Object[] oldState = (Object[])context.getRenderKit().getResponseStateManager().getState(context, viewToRender.getViewId());
      String oldStateStr = oldState.length > 0 ? (String)oldState[0] : null;

      Object[] newState = (Object[]) context.getApplication().getStateManager().saveView(context);
      String newStateStr = newState.length > 0 ? (String) newState[0] : null;

      replaceState(context, oldStateStr, newStateStr);

    } else {
      super.renderView(context, viewToRender);
    }
  }

  @Override
  public Locale calculateLocale(FacesContext context) {
    Locale l = (Locale) context.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if (l == null)
      return super.calculateLocale(context);
    return l;
  }

  private class LoggedWriter extends ResponseWriter {

    ResponseWriter w;

    private LoggedWriter(ResponseWriter w) {
      this.w = w;
    }

    @Override
    public void write(int c) throws IOException {
      w.write(c);
    }

    @Override
    public void write(char[] cbuf) throws IOException {
      System.out.println(new String(cbuf));
      w.write(cbuf);
    }

    @Override
    public void write(char[] cbuf, int off, int len) throws IOException {
      System.out.println(new String(cbuf).subSequence(off, off + len));
      w.write(cbuf, off, len);
    }

    @Override
    public void write(String str) throws IOException {
      System.out.println(str);
      w.write(str);
    }

    @Override
    public void write(String str, int off, int len) throws IOException {
      System.out.println(str.subSequence(off, off + len));
      w.write(str, off, len);
    }

    @Override
    public Writer append(CharSequence csq) throws IOException {
      return w.append(csq);
    }

    @Override
    public Writer append(CharSequence csq, int start, int end) throws IOException {
      return w.append(csq, start, end);
    }

    @Override
    public Writer append(char c) throws IOException {
      return w.append(c);
    }

    @Override
    public void close() throws IOException {
      w.close();
    }

    @Override
    public void startDocument() throws IOException {
      w.startDocument();
    }

    @Override
    public void endDocument() throws IOException {
      w.endDocument();
    }

    @Override
    public void startElement(String s, UIComponent uiComponent) throws IOException {
      w.startElement(s, uiComponent);
    }

    @Override
    public void endElement(String s) throws IOException {
      w.endElement(s);
    }

    @Override
    public void writeAttribute(String s, Object o, String s1) throws IOException {
      w.writeAttribute(s, o, s1);
    }

    @Override
    public void flush() throws IOException {
      w.flush();
    }

    @Override
    public String getCharacterEncoding() {
      return w.getCharacterEncoding();
    }

    @Override
    public String getContentType() {
      return w.getContentType();
    }

    @Override
    public void writeURIAttribute(String s, Object o, String s1) throws IOException {
      w.writeURIAttribute(s, o, s1);
    }

    @Override
    public void writeComment(Object o) throws IOException {
      w.writeComment(o);
    }

    @Override
    public void writeText(Object o, String s) throws IOException {
      w.writeText(o, s);
    }

    @Override
    public void writeText(char[] chars, int i, int i1) throws IOException {
      w.writeText(chars, i, i1);
    }

    @Override
    public ResponseWriter cloneWithWriter(Writer writer) {
      return w.cloneWithWriter(writer);
    }

    @Override
    public void writeText(Object object, UIComponent component, String string) throws IOException {
      w.writeText(object, component, string);
    }
  }

}
