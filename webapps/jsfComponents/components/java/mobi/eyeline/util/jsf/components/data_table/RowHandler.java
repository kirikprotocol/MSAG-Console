package mobi.eyeline.util.jsf.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;
import mobi.eyeline.util.jsf.components.data_table.model.ModelWithObjectIds;

import javax.el.ELException;
import javax.el.VariableMapper;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RowHandler extends ComponentHandler {

  private final TagAttribute innerData;
  private final TagAttribute innerRows;
  private final TagAttribute opened;

  public RowHandler(ComponentConfig config) {
    super(config);

    innerData = getAttribute("innerData");
    innerRows = getAttribute("innerRows");
    opened = getAttribute("opened");
  }

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {

    Row r = (Row) c;
    VariableMapper variableMapper = ctx.getVariableMapper();
    String tid = (String) variableMapper.resolveVariable("___tid").getValue(ctx);

    if (variableMapper.resolveVariable(tid + "___currentRow") == null) {
      r.setHeader(true);

    } else {

      r.setInner(variableMapper.resolveVariable(tid + "___innerRow") != null);
      String var = (String) variableMapper.resolveVariable(tid + "___var").getValue(ctx);

      Object currentRow = variableMapper.resolveVariable(tid + "___currentRow").getValue(ctx);
      r.setVar(ctx, currentRow);

      variableMapper.setVariable(var, r.getVarExpr());
      if (!r.isInner()) {
        DataTable t = (DataTable) variableMapper.resolveVariable(tid + "___dataTable").getValue(ctx);
        DataTableModel dt = t.getModel();
        String rowId = null;
        if(dt instanceof ModelWithObjectIds) {
          try {
            rowId = ((ModelWithObjectIds)dt).getId(r.getVarExpr().getValue(ctx));
          } catch (ModelException e) {
            t.setError(e);
            return;
          }
        }
        r.setRowId(rowId != null ? rowId :  getId(ctx));
        if (innerData != null)
          r.setHasInnerData(innerData.getBoolean(ctx));
        if (opened != null)
          r.setOpened(opened.getBoolean(ctx));
        if (innerRows != null) {
          List innerRowsList = (List) innerRows.getObject(ctx);
          if (innerRowsList != null && !innerRowsList.isEmpty()) {
            r.setHasInnerRows(true);
            variableMapper.setVariable(tid + "___innerRows", new ConstantExpression(innerRowsList));
            variableMapper.setVariable(tid + "___innerRowsId", new ConstantExpression(r.getRowId()));
          } else {
            variableMapper.setVariable(tid + "___innerRows", null);
            variableMapper.setVariable(tid + "___innerRowsId", null);
          }
        }
      } else {
        r.setRowId((String) variableMapper.resolveVariable(tid + "___innerRowsId").getValue(ctx));
      }
    }

    nextHandler.apply(ctx, c);
  }
}
