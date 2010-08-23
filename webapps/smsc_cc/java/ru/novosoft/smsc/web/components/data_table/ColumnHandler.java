package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
public class ColumnHandler extends ComponentHandler {

  private final TagAttribute name;
  private final TagAttribute title;
  private final TagAttribute width;
  private final TagAttribute align;
  private final TagAttribute sortable;

  public ColumnHandler(ComponentConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getRequiredAttribute("title");
    width = getAttribute("width");
    sortable = getAttribute("sortable");
    align = getAttribute("align");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    Column c = new Column();
    c.setName(name.getValue(ctx));
    c.setTitle(title.getValue(ctx));
    if (width != null)
      c.setWidth(width.getValue(ctx));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(ctx));
    if (align != null)
      c.setAlign(align.getValue(ctx));

    return c;
  }
  
}
