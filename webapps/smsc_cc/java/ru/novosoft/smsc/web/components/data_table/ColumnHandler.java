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
  private final TagAttribute defaultSortOrder;

  public ColumnHandler(ComponentConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getRequiredAttribute("title");
    width = getAttribute("width");
    sortable = getAttribute("sortable");
    align = getAttribute("align");
    defaultSortOrder = getAttribute("defaultSortOrder");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    Column  c = new Column();
    c.setName(name.getValue(ctx));
    c.setTitle(title.getValue(ctx));
    if (width != null)
      c.setWidth(width.getValue(ctx));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(ctx));
    if (align != null)
      c.setAlign(align.getValue(ctx));
    if (defaultSortOrder != null)
      c.setDefaultSortOrder(defaultSortOrder.getValue());

    return c;
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent component) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {

    Column c = (Column)component;

    c.setName(name.getValue(ctx));
    c.setTitle(title.getValue(ctx));
    if (width != null)
      c.setWidth(width.getValue(ctx));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(ctx));
    if (align != null)
      c.setAlign(align.getValue(ctx));
    if (defaultSortOrder != null)
      c.setDefaultSortOrder(defaultSortOrder.getValue());


    nextHandler.apply(ctx, c);
  }



}
