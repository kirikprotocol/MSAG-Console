package ru.novosoft.smsc.web.components.paged_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.FaceletException;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

/**
 * @author Artem Snopkov
 */
public class ImageColumnTagHandler extends TagHandler {

  private final TagAttribute name;
  private final TagAttribute title;
  private final TagAttribute actionListener;
  private final TagAttribute width;
  private final TagAttribute valueMapping;
  private final TagAttribute sortable;

  public ImageColumnTagHandler(TagConfig config) {
    super(config);

    name = getRequiredAttribute("name");
    title = getRequiredAttribute("title");
    actionListener = getAttribute("actionListener");
    width= getAttribute("width");
    valueMapping = getRequiredAttribute("valueMapping") ;
    sortable = getAttribute("sortable");
  }

  public void apply(FaceletContext faceletContext, UIComponent uiComponent) throws IOException, FacesException, FaceletException, ELException {
    ImageColumn c = new ImageColumn(name.getValue(faceletContext), title.getValue(faceletContext));
    if (actionListener != null)
      c.setActionListener(actionListener.getValue());
    if (width != null)
      c.setWidth(width.getInt(faceletContext));
    if (sortable != null)
      c.setSortable(sortable.getBoolean(faceletContext));


    Map<String, String> valueMap = new HashMap<String, String>();

    String valueMappingStr = valueMapping.getValue();
    StringTokenizer st = new StringTokenizer(valueMappingStr, ";");
    while (st.hasMoreTokens()) {
      String token = st.nextToken().trim();
      int i= token.indexOf(':');
      String key = token.substring(0, i);
      String value = token.substring(i+1);
      valueMap.put(key, value);
    }

    c.setValueMapping(valueMap);

    ((PagedTable)uiComponent).addColumn(c);
  }
}
