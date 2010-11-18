package mobi.eyeline.informer.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RowHandler extends ComponentHandler {

  private final TagAttribute rowId;
  private final TagAttribute data;
  private final TagAttribute innerData;
  private final TagAttribute innerRows;
  private final TagAttribute opened;

  public RowHandler(ComponentConfig config) {
    super(config);

    rowId = getAttribute("rowId");
    data = getAttribute("data");
    innerData = getAttribute("innerData");
    innerRows = getAttribute("innerRows");
    opened = getAttribute("opened");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    return new Row();
  }

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {

    Row r = (Row) c;
    String tid = (String) ctx.getVariableMapper().resolveVariable("___tid").getValue(ctx);

    if (ctx.getVariableMapper().resolveVariable(tid + "___currentRow") == null) {
      r.setHeader(true);

    } else {

      r.setInner(ctx.getVariableMapper().resolveVariable(tid + "___innerRow") != null);
      String var = (String) ctx.getVariableMapper().resolveVariable(tid + "___var").getValue(ctx);

      Object currentRow = ctx.getVariableMapper().resolveVariable(tid + "___currentRow").getValue(ctx);
      r.setVar(ctx, currentRow);

      ctx.getVariableMapper().setVariable(var, r.getVarExpr());
      if (!r.isInner()) {
        if (rowId != null)
          r.setRowId(rowId.getValue(ctx));
        else
          r.setRowId(getId(ctx));
        if (innerData != null)
          r.setHasInnerData(innerData.getBoolean(ctx));
        if (opened != null)
          r.setOpened(opened.getBoolean(ctx));
        if (innerRows != null) {
          List innerRowsList = (List) innerRows.getObject(ctx);
          if (innerRowsList != null && !innerRowsList.isEmpty()) {
            r.setHasInnerRows(true);
            ctx.getVariableMapper().setVariable(tid + "___innerRows", new ConstantExpression(innerRowsList));
            ctx.getVariableMapper().setVariable(tid + "___innerRowsId", new ConstantExpression(r.getRowId()));
          } else {
            ctx.getVariableMapper().setVariable(tid + "___innerRows", null);
            ctx.getVariableMapper().setVariable(tid + "___innerRowsId", null);
          }
        }
      } else {
        r.setRowId((String) ctx.getVariableMapper().resolveVariable(tid + "___innerRowsId").getValue(ctx));
      }
    }

    nextHandler.apply(ctx, c);
  }
}
