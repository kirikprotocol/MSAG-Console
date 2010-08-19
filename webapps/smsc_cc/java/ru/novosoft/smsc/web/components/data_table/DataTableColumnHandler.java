package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
public class DataTableColumnHandler extends ComponentHandler {

  private final TagAttribute name;
  private final TagAttribute title;
  private final TagAttribute width;
  private final TagAttribute align;
  private final TagAttribute sortable;
  private final TagAttribute var;
  private final TagAttribute rowIndexVar;

  public DataTableColumnHandler(ComponentConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getRequiredAttribute("title");
    width = getAttribute("width");
    sortable = getAttribute("sortable");
    var = getAttribute("var");
    rowIndexVar = getAttribute("rowIndexVar");
    align = getAttribute("align");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    DataTableColumn c = new DataTableColumn();
    c.setName(name.getValue(ctx));
    c.setTitle(title.getValue(ctx));
    if (width != null)
      c.setWidth(width.getValue(ctx));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(ctx));
    if (var != null)
      c.setVar(var.getValue());
    if (rowIndexVar != null)
      c.setRowNumVar(rowIndexVar.getValue());
    if (align != null)
      c.setAlign(align.getValue(ctx));

    return c;
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    DataTableColumn column = (DataTableColumn) c;

    DataTable t = (DataTable)ctx.getVariableMapper().resolveVariable("dataTable").getValue(ctx);
    column.setRow(t.getCurrentRow());
    column.setRowNum(t.getCurrentRowNum());

    ConstantExpression varExpression = column.getVarExpression();
    if (varExpression == null) {
      varExpression = new ConstantExpression(t.getCurrentRow().getData(column.getName()));
      column.setVarExpression(varExpression);
      ctx.getVariableMapper().setVariable(column.getVar(), varExpression);
    } else
      varExpression.setValue(ctx, t.getCurrentRow().getData(column.getName()));

    ConstantExpression rowNumExpression = column.getRowNumExpression();
    if (rowNumExpression == null) {
      rowNumExpression = new ConstantExpression(t.getCurrentRowNum());
      column.setRowNumExpression(rowNumExpression);
      ctx.getVariableMapper().setVariable(column.getRowNumVar(), rowNumExpression);
    } else
      rowNumExpression.setValue(ctx, t.getCurrentRowNum());

    nextHandler.apply(ctx, c);
  }

}
