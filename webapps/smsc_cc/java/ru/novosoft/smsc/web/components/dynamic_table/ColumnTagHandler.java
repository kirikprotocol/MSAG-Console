package ru.novosoft.smsc.web.components.dynamic_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
abstract class ColumnTagHandler extends TagHandler {

  protected final TagAttribute name;
  protected final TagAttribute width;
  protected final TagAttribute title;

  public ColumnTagHandler(TagConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getAttribute("title");
    width = getAttribute("width");
  }

  protected void apply(Column column, FaceletContext ctx, UIComponent parent) {
    column.setName(name.getValue(ctx));
    if (title != null)
      column.setTitle(title.getValue(ctx));
    if (width != null)
      column.setWidth(width.getInt(ctx));
  }
}
