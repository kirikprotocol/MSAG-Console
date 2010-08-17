package ru.novosoft.smsc.web.components.paged_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.FaceletException;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TextColumnTagHandler extends TagHandler {

  private final TagAttribute name;
  private final TagAttribute title;
  private final TagAttribute actionListener;
  private final TagAttribute width;
  private final TagAttribute sortable;
  private final TagAttribute textColor;

  public TextColumnTagHandler(TagConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getRequiredAttribute("title");
    actionListener = getAttribute("actionListener");
    width= getAttribute("width");
    sortable = getAttribute("sortable");
    textColor=getAttribute("textColor");
  }

  public void apply(FaceletContext faceletContext, UIComponent uiComponent) throws IOException, FacesException, FaceletException, ELException {
    TextColumn c = new TextColumn(name.getValue(faceletContext), title.getValue(faceletContext));
    if (actionListener != null)
      c.setActionListener(actionListener.getValue());
    if (width != null)
      c.setWidth(width.getInt(faceletContext));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(faceletContext));
    if (textColor != null)
      c.setTextColor(textColor.getValue());
    ((PagedTable)uiComponent).addColumn(c);
  }
}
