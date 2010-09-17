package mobi.eyeline.informer.web.components.dynamic_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TextColumnTagHandler extends ColumnTagHandler {

  private final TagAttribute allowEditAfterAdd;
  private final TagAttribute maxLength;

  public TextColumnTagHandler(TagConfig config) {
    super(config);

    allowEditAfterAdd = getAttribute("allowEditAfterAdd");
    maxLength = getAttribute("maxLength");
  }

  public void apply(FaceletContext faceletContext, UIComponent parent) throws IOException, FacesException, ELException {
    if (parent instanceof DynamicTable) {
      TextColumn textColumn = new TextColumn();
      apply(textColumn, faceletContext, parent);
      if (allowEditAfterAdd != null)
        textColumn.setAllowEditAfterAdd(allowEditAfterAdd.getBoolean(faceletContext));
      if (maxLength != null)
        textColumn.setMaxLength(maxLength.getInt(faceletContext));

      ((DynamicTable) parent).addColumn(textColumn);
    }
  }
}
