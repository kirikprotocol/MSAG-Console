package mobi.eyeline.util.jsf.components.downloading;

import javax.el.ELContext;
import javax.el.MethodExpression;
import javax.el.ValueExpression;
import javax.faces.component.StateHolder;
import javax.faces.context.FacesContext;
import javax.faces.event.AbortProcessingException;
import javax.faces.event.ActionEvent;
import javax.faces.event.ActionListener;
import javax.servlet.http.HttpServletResponse;
import java.io.OutputStream;
import java.util.Map;


public class FileDownloadListener implements ActionListener, StateHolder {

  private boolean trans;

  private Object filename;

  private Object contentType;

  private MethodExpression method;

  public void processAction(ActionEvent actionEvent) throws AbortProcessingException {
    FacesContext context = FacesContext.getCurrentInstance();
    ELContext elContext = context.getELContext();
    String filename = getFilename(elContext);
    String contentType = getContentType(elContext);

    HttpServletResponse hsr = (HttpServletResponse) context.getExternalContext().getResponse();
    try {
      if (contentType != null) {
        hsr.setContentType(contentType);
      }
      if (filename != null) {
        // check for supported user agents. Currently IE, Gecko, and WebKit.
        // IE and WebKit use UTF-8 encoding.
        boolean isGecko = true;
        Map<String, String> headers = context.getExternalContext().getRequestHeaderMap();
        String agentName = headers.get("User-Agent").toLowerCase();
        if (agentName.contains("msie") || agentName.contains("applewebkit") || agentName.contains("safari")) {
          isGecko = false;
        }
//        boolean isIE = CoreRenderer.isIE(RenderingContext.getCurrentInstance());
        String encodeHTTPHeaderFilename = MimeUtility.encodeHTTPHeader(filename, !isGecko);
        // double quotes are needed in case the filename is long. otherwise the filename gets
        // truncated in Firefox.
        hsr.setHeader("Content-Disposition", "attachment; filename=\""+encodeHTTPHeaderFilename + "\"");

      }
      MethodExpression method = getMethod();
      OutputStream out = null;

      try {
        out = hsr.getOutputStream();
        method.invoke(context.getELContext(), new Object[]{context, out});
      }finally {
        if(out != null) {
          try{
            out.close();
          }catch (Exception ignored){}
        }
      }
      context.responseComplete();
    }
    catch (Exception e){
      hsr.reset();
      throw new AbortProcessingException(e);
    }
  }

  public Object saveState(FacesContext facesContext) {
    Object[] values = new Object[3];
    values[0] = filename;
    values[1] = contentType;
    values[2] = method;
    return values;
  }

  public void restoreState(FacesContext facesContext, Object o) {
    Object[] values = (Object[]) o;
    filename = values[0];
    contentType = values[1];
    method = (MethodExpression)values[2];
  }

  public boolean isTransient() {
    return trans;
  }

  public void setTransient(boolean b) {
    this.trans = b;
  }

  public void setFilename(Object filename) {
    this.filename = filename;
  }

  public void setContentType(Object contentType) {
    this.contentType = contentType;
  }

  public String getFilename(ELContext context) {
    return getStringValue(filename, context);
  }

  public String getContentType(ELContext context) {
    return getStringValue(contentType, context);
  }

  public MethodExpression getMethod() {
    return method;
  }

  public void setMethod(MethodExpression method) {
    this.method = method;
  }


  private String getStringValue(Object o, ELContext context) {
    if(o == null) {
      return null;
    }
    if(o instanceof  ValueExpression) {
      return ((ValueExpression) o).getValue(context).toString();
    }else {
      return o.toString();
    }
  }
}
