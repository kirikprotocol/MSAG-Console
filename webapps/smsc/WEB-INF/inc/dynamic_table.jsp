<%@ page import="java.util.Iterator"%>
<%
  if (tableHelper instanceof ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper) {
    final String tableUid = tableHelper.getUid();
    if (request.getAttribute("properties_table_js_included") == null) {%>
<script src="/scripts/dynamic_table.js" type="text/javascript"></script>
<%    request.setAttribute("properties_table_js_included", new Object());
    }%>
  <input type=hidden id="<%=tableHelper.getTotalCountPrefix()%>" name="<%=tableHelper.getTotalCountPrefix()%>" value="<%=tableHelper.getRowsCount()%>">
  <% if (tableHelper.isShowTableTitle()) { %>
  <div class=page_subtitle><%=getLocString(tableHelper.getName())%></div>
  <% } %>
  <table class="properties_list" cellspacing=0 id="<%=tableUid%>" width="30%">
<!-- --------------------------------------- COLUMNS HEADERS --------------------------------------------------- -->
<% if (tableHelper.isShowColumnsTitle()) {%>
    <thead>
      <tr>
<%for (Iterator iter = tableHelper.getColumns(); iter.hasNext();) {%>
        <td><%=getLocString(((ru.novosoft.smsc.jsp.util.helper.dynamictable.Column)iter.next()).getName())%></td>
<%}%>
        <td>&nbsp;</td>
      </tr>
    </thead>
<%}%>
<!-- ------------------------------------------- TABLE BODY ---------------------------------------------------- -->
    <tbody>
<%=ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableRenderer.renderTableBody(tableHelper)%>
    </tbody>
  </table>
<%} else {%>
  Invalid tableHelper type!!! tableHelper must extends PropertiesTableHelper
<%}%>