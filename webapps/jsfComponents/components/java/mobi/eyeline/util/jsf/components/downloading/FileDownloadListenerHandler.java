package mobi.eyeline.util.jsf.components.downloading;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;
import com.sun.facelets.tag.jsf.ComponentSupport;

import javax.el.ELException;
import javax.el.MethodExpression;
import javax.el.ValueExpression;
import javax.faces.FacesException;
import javax.faces.component.ActionSource;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.io.IOException;
import java.io.OutputStream;


public class FileDownloadListenerHandler extends TagHandler {

  private static final Class[] _METHOD_PARAMS = new Class[]{FacesContext.class, OutputStream.class};

  private final TagAttribute _method;
  private final TagAttribute _filename;
  private final TagAttribute _contentType;

  public FileDownloadListenerHandler(TagConfig tagConfig) {
    super(tagConfig);
    _filename = getAttribute("filename");
    _contentType = getAttribute("contentType");
    _method = getRequiredAttribute("method");
  }

  public void apply(FaceletContext faceletContext, UIComponent parent) throws IOException, FacesException, ELException {
    if(ComponentSupport.isNew(parent)) {
      FileDownloadListener listener = new FileDownloadListener();
      if (_filename != null) {
        if (_filename.isLiteral()) {
          listener.setFilename(_filename.getValue());
        } else {
          ValueExpression valueExp = _filename.getValueExpression(faceletContext, Object.class);
          listener.setFilename(valueExp);
        }
      }

      if (_contentType != null) {
        if (_contentType.isLiteral()) {
          listener.setContentType(_contentType.getValue());
        } else {
          ValueExpression valueExp = _contentType.getValueExpression(faceletContext, Object.class);
          listener.setContentType(valueExp);
        }
      }

      MethodExpression me = _method.getMethodExpression(faceletContext, Object.class,_METHOD_PARAMS);
      listener.setMethod(me);

      ActionSource actionSource = (ActionSource)parent;
      actionSource.addActionListener(listener);
    }
  }
}