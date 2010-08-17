package ru.novosoft.smsc.web.components.paged_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.el.ELException;
import javax.el.MethodExpression;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import javax.faces.event.ActionEvent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class PagedTableHandler extends ComponentHandler {

  private final TagAttribute value;
  private final TagAttribute autoUpdate;
  private final TagAttribute pageSize;

  public PagedTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    autoUpdate = getAttribute("autoUpdate");
    pageSize = getAttribute("pageSize");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    PagedTable t = new PagedTable();
    t.setModel((PagedTableModel) value.getObject(ctx, PagedTableModel.class));
    if (autoUpdate != null)
      t.setAutoUpdate(autoUpdate.getInt(ctx));
    if (pageSize != null)
      t.setPageSize(pageSize.getInt(ctx));
    return t;
  }

  @Override
  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws IOException, FacesException, ELException {

    PagedTable t = (PagedTable) c;

    if (t.getSelectedRows().size() > 0) {
      int[] rows = new int[t.getSelectedRows().size()];
      for (int i = 0; i < rows.length; i++)
        rows[i] = t.getSelectedRows().get(i);
      t.getModel().setSelectedRows(rows);
    }

    if (t.getSelectedColumn() != null) {
      for (Column column : t.getColumns()) {
        if (column.getName().equals(t.getSelectedColumn())) {
          MethodExpression m = ctx.getExpressionFactory().createMethodExpression(ctx, column.getActionListener(), Void.class, new Class[]{ActionEvent.class});
          if (m != null)
            m.invoke(ctx, new Object[]{new SelectElementEvent(t, t.getSelectedColumn(), t.getSelectedRow())});
        }
      }

    }

    nextHandler.apply(ctx, c);
  }
}
