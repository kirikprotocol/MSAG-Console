package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class RowHandler extends ComponentHandler {

  private final TagAttribute rowId;
  private final TagAttribute data;
  private final TagAttribute innerData;
  private final TagAttribute opened;

  public RowHandler(ComponentConfig config) {
    super(config);

    rowId = getAttribute("rowId");
    data = getAttribute("data");
    innerData = getAttribute("innerData");
    opened = getAttribute("opened");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    return new Row();
  }

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {

    if (ctx.getVariableMapper().resolveVariable("___currentRow") != null) {
      Row r = (Row) c;
      DataTableRow currentRow = (DataTableRow) ctx.getVariableMapper().resolveVariable("___currentRow").getValue(ctx);

      r.setRow(currentRow);
      r.setInner(ctx.getVariableMapper().resolveVariable("___innerRow") != null);
      r.setRowId(ctx, currentRow.getId());
      r.setData(ctx, currentRow.getData());
      r.setInnerData(ctx, currentRow.getInnerData());

      if (rowId != null)
        ctx.getVariableMapper().setVariable(rowId.getValue(), r.getRowIdExpr());
      if (data != null)
        ctx.getVariableMapper().setVariable(data.getValue(), r.getDataExpr());
      if (innerData != null)
        ctx.getVariableMapper().setVariable(innerData.getValue(), r.getInnerDataExpr());
      if (opened != null)
        r.setOpened(opened.getBoolean(ctx));
    }
    
    nextHandler.apply(ctx, c);
  }
}
