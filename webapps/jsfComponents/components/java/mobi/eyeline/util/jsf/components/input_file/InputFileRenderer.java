package mobi.eyeline.util.jsf.components.input_file;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.ConverterException;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.Map;

/**
 * author: Aleksandr Khalitov
 */
public class InputFileRenderer extends Renderer {

  private String getErrorMessage(FacesContext context, UIComponent t) {
    Iterator messages = context.getMessages(t.getClientId(context));
    if (messages != null && messages.hasNext()) {
      FacesMessage m = (FacesMessage) messages.next();
      String text = m.getDetail();
      if (text == null || text.length() == 0)
        text = m.getSummary();
      return text;
    }
    return null;
  }

  @Override
  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    InputFile inputFile = (InputFile)component;
    String inputClass = "";
    if (!inputFile.isValid())
      inputClass = "class=\"validationError\"";

    String id = inputFile.getId();

    String ss = (String)inputFile.getAttributes().get("size");
    Integer size = null;
    if(ss != null && ss.length()>0) {
      try{
        size = Integer.parseInt(ss);
      }catch (NumberFormatException ignored){}
    }

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("<div class=\"eyeline_inputFile\" id=\"").a(id).a(".div\">");

    w.a("<input type=\"file\" id=\"").a(id).a("\" name=\"").a(id).a("\" ").a(inputClass);
    if(size != null) {
      w.a(" size=\"").a(size.toString()).a("\"");
    }
    w.a('>');

    if (!inputFile.isValid()) {
      String text = getErrorMessage(context, inputFile);
      if (text != null)
        w.a("<span class=\"error\" title=\"").a(text).a("\"> (!)</span>");
    }
    w.a("</div>");
  }


  @Override
  public Object getConvertedValue(FacesContext context, UIComponent component, final Object files) throws ConverterException {
    if(files == null) {
      return null;
    }
    final Object file =  ((Object[])files)[0];
    if(file == null) {
      return null;
    }
    try{
      final String contentType = (String)file.getClass().getMethod("getContentType").invoke(file);
      final Long lenght;
      if(!file.getClass().getName().equals("org.apache.commons.fileupload.disk.DiskFileItem")) {
        lenght = (Long)file.getClass().getMethod("getLength").invoke(file);
      }else {
        lenght = (Long)file.getClass().getMethod("getSize").invoke(file);
      }
      final Method getIS = file.getClass().getMethod("getInputStream");
      return new UploadedFile() {
        public String getContentType() {
          return contentType;
        }
        public long getLength() {
          return lenght;
        }
        public InputStream getInputStream() throws Exception {
          try {
            return (InputStream)getIS.invoke(file);
          } catch (Exception e){
            throw new RuntimeException(e);
          }
        }
      };
    }catch (Exception e) {
      throw new ConverterException();
    }
  }

  @Override
  public void decode(FacesContext context, UIComponent component) {
    try{
      Object file = decodeTrinidadFile(context);
      if(file == null) {
        file = decodeTomahawk(context);
      }
      ((InputFile)component).setSubmittedValue(new Object[]{file});
    }catch (Exception e) {
      throw new IllegalArgumentException(e);
    }
  }

  @SuppressWarnings({"unchecked"})
  private Object decodeTrinidadFile(FacesContext context) throws Exception{
    Map<String, Object> reqParams = context.getExternalContext().getRequestMap();
    Object o = reqParams.get("org.apache.myfaces.trinidadinternal.webapp.UploadedFiles");
    if(o != null && o.getClass().getName().equals("org.apache.myfaces.trinidadinternal.config.upload.UploadedFiles")) {
      Iterator<String> i = (Iterator<String>)o.getClass().getMethod("getUploadedNames").invoke(o);
      if(i != null) {
        if(i.hasNext()) {
          String name = i.next();
          return o.getClass().getMethod("getUploadedFile", java.lang.String.class).invoke(o, (Object[])new String[]{name});
        }
      }
    }
    return null;
  }

  private Object decodeTomahawk(FacesContext context) throws Exception{
    Object r = context.getExternalContext().getRequest();
    if(r.getClass().getName().equals("org.apache.myfaces.webapp.filter.MultipartRequestWrapper")) {
      Map items = (Map)r.getClass().getMethod("getFileItems").invoke(r);
      if(items != null) {
        Iterator i = items.values().iterator();
        if(i.hasNext()) {
          return  i.next();
        }
      }
    }
    return null;
  }

}
