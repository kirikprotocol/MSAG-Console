package ru.novosoft.smsc.web.components.paged_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.FaceletException;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class FilterTagHandler extends TagHandler {

  public FilterTagHandler(TagConfig config) {
    super(config);
  }

  public void apply(FaceletContext faceletContext, UIComponent uiComponent) throws IOException, FacesException, FaceletException, ELException {
    ((PagedTable)uiComponent).setHasFilter(true);
    nextHandler.apply(faceletContext, uiComponent);
  }
}
