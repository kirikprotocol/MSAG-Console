package ru.novosoft.smsc.web.components.dynamic_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class SelectColumnTagHandler extends ColumnTagHandler {

  private final TagAttribute allowEditAfterAdd;
  private final TagAttribute values;
  private final TagAttribute uniqueValues;

  public SelectColumnTagHandler(TagConfig config) {
    super(config);

    allowEditAfterAdd = getAttribute("allowEditAfterAdd");
    values = getRequiredAttribute("values");
    uniqueValues = getAttribute("uniqueValues");
  }

  public void apply(FaceletContext faceletContext, UIComponent parent) throws IOException, FacesException, ELException {
    if (parent instanceof DynamicTable) {
      SelectColumn column = new SelectColumn();
      apply(column, faceletContext, parent);

      List v = (List) values.getObject(faceletContext, List.class);
      List<String> values = new ArrayList<String>(v.size());
      for (Object val : v)
        values.add(val.toString());

      column.setValues(values);

      if (allowEditAfterAdd != null)
        column.setAllowEditAfterAdd(allowEditAfterAdd.getBoolean(faceletContext));

      if (uniqueValues != null)
        column.setUniqueValues(uniqueValues.getBoolean(faceletContext));

      ((DynamicTable) parent).addColumn(column);
    }
  }
}