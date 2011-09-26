package mobi.eyeline.util.jsf.components.dynamic_table;

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
  protected final TagAttribute columnClass;
  protected final TagAttribute title;

  public ColumnTagHandler(TagConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getAttribute("title");
    columnClass = getAttribute("columnClass");
  }

  protected void apply(Column column, FaceletContext ctx, UIComponent parent) {
    column.setName(name.getValue(ctx));
    if (title != null)
      column.setTitle(title.getValue(ctx));
    if (columnClass != null)
      column.setColumnClass(columnClass.getValue(ctx));
  }
}
