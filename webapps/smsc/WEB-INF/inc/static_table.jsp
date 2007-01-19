<%@ page import="java.util.Iterator"%>

<%
  if (tableHelper instanceof ru.novosoft.smsc.jsp.util.helper.statictable.StaticTableHelper) {
    int rowNumber = 0;
    if (request.getAttribute("base_table_js_included") == null) {%>
<script src="/scripts/static_table.js" type="text/javascript"></script>
<%    request.setAttribute("base_table_js_included", new Object()); }%>
<input type=hidden id="<%=tableHelper.getSelectedCellIdPrefix()%>" name="<%=tableHelper.getSelectedCellIdPrefix()%>">
<input type=hidden id="<%=tableHelper.getSelectedColumnIdPrefix()%>" name="<%=tableHelper.getSelectedColumnIdPrefix()%>">
<input type=hidden id="<%=tableHelper.getSortedColumnIdPrefix()%>" name="<%=tableHelper.getSortedColumnIdPrefix()%>">
<%  for (int i = 0; i < tableHelper.getSortOrder().length; i++) {
      final ru.novosoft.smsc.jsp.util.helper.statictable.StaticTableHelper.SortOrderElement element = tableHelper.getSortOrder()[i];
      if (element != null) {%>
<input type=hidden name="<%=tableHelper.getSortOrderPrefix() + String.valueOf(i)%>" value="<%=(element.getOrderType() == ru.novosoft.smsc.jsp.util.helper.statictable.OrderType.ASC ? "" : "-") + element.getColumnId()%>">
<%    }
    }%>
<table class=list cellspacing=0>
    <!-- -------------------------------------- COLUMN WIDTH ------------------------------------------------------- -->
    <% for (Iterator iter = tableHelper.getColumns(); iter.hasNext();) { %>
  <col width="<%=((ru.novosoft.smsc.jsp.util.helper.statictable.Column)iter.next()).getWidth()%>%"><% } %>
    <!-- --------------------------------------- COLUMNS HEADERS --------------------------------------------------- -->
  <thead>
    <tr>
    <% // SHOW COLUMNS HEADER
      for (Iterator iter = tableHelper.getColumns(); iter.hasNext();) {
        final ru.novosoft.smsc.jsp.util.helper.statictable.Column column = (ru.novosoft.smsc.jsp.util.helper.statictable.Column)iter.next(); %>
      <th><%=ru.novosoft.smsc.jsp.util.helper.statictable.StaticTableRenderer.renderColumn(tableHelper, column, tableHelper.getOrderType(column.getId()))%></th>
    <%} %>
    </tr>
  </thead>
    <!-- ------------------------------------------- TABLE BODY ---------------------------------------------------- -->
  <tbody>
    <%for (Iterator rows = tableHelper.getRows(); rows.hasNext();) {
        final ru.novosoft.smsc.jsp.util.helper.statictable.Row row = (ru.novosoft.smsc.jsp.util.helper.statictable.Row)rows.next(); %>
    <tr class=row<%=rowNumber++&1%>>
    <%  for (Iterator cols = tableHelper.getColumns(); cols.hasNext();) {
          final ru.novosoft.smsc.jsp.util.helper.statictable.Column column = (ru.novosoft.smsc.jsp.util.helper.statictable.Column)cols.next();
          final ru.novosoft.smsc.jsp.util.helper.statictable.Cell cell = row.getCell(column); %>
        <%=ru.novosoft.smsc.jsp.util.helper.statictable.StaticTableRenderer.renderCell(tableHelper, cell, column)%>
    <%  }%>
    </tr>
    <%}%>
  </tbody>
</table>
<%} else {%>
  Invalid tableHelper type!!! tableHelper must extends BaseTableHelper
<% } %>
