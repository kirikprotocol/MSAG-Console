<%@ page import = "ru.novosoft.smsc.jsp.util.tables.*"%>
<%! 
void showTable(JspWriter out, QueryResultSet results)
  throws java.io.IOException
{
  String[] columns = results.getColumns();
  String sort = (String)results.getSortOrder().get(0);
  if (sort == null) sort = "";
  out.println("<table class=\"list\" cellspacing=\"0\"><thead class=\"list\"><tr class=\"list\">");
  for (int i = 0; i < columns.length; i++)
  {
    out.println("  <th class=\"list\" width=\"" + (99/columns.length)+ "%\"><a href=\"?sort=" + (sort.equals(columns[i]) ? URLEncoder.encode("-" + columns[i]) : URLEncoder.encode(columns[i])) + "\">" + StringEncoderDecoder.encode(columns[i]) + "</a></th>");
  }
  out.println("  <th class=\"list\" width=\"1%\"><a href=\"edit.jsp\">Add</a></th>");
  out.println("</tr></thead>");
  for (Iterator i = results.iterator(); i.hasNext(); )
  {
    DataItem item = (DataItem) i.next();
    out.println("  <tr class=\"list\">");
    String params = "?";
    for (int j = 0; j < columns.length; j++) {
      Object value = item.getValue(columns[j]);
      if (value instanceof Vector) {
        out.println("    <td class=\"list\">");
        for (Iterator k = ((Vector) value).iterator(); k.hasNext(); )
        {
          out.println(StringEncoderDecoder.encode((String)k.next()) + (k.hasNext() ? "<br>" : ""));
        }
        out.println("</td>");
      } else if (value instanceof Boolean)
	  {
	  	out.println("    <td class=\"list\" style=\"width:1%\"><input disabled type=\"Checkbox\" " + (((Boolean)value).booleanValue() ? "checked" : "")+"/></td>");
        params += URLEncoder.encode(columns[j]) + '=' + URLEncoder.encode(((Boolean)value).toString()) + '&';
	  } else {
        out.println("    <td class=\"list\">" + StringEncoderDecoder.encode((String)value) + "</td>");
        params += URLEncoder.encode(columns[j]) + '=' + URLEncoder.encode((String)value) + '&';
      }
    }
    out.println("  <td class=\"list\"><a href=\"edit.jsp" + params + "\">Edit</a>&nbsp;<a href=\"delete.jsp" + params + "\">Delete</a></td>");
    out.println("  </tr>");
  }
  out.println("</table>");
}
%>