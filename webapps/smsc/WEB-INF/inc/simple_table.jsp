<%@ page import="java.util.Iterator,
                 ru.novosoft.smsc.jsp.util.tables.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.table.*"%>

<%
  if (bean instanceof SimpleTableBean) {
%>

<input type=hidden name=selectedCellId>
<input type=hidden name=selectedColumnId>

<%
    for (int i = 0; i < bean.getSortOrder().length; i++) {
      final SimpleTableBean.SortOrderElement element = bean.getSortOrder()[i];
      if (element != null) {%>
<input type=hidden name="<%=SimpleTableBean.SORT_ORDER_PREFIX + String.valueOf(i)%>" value="<%=(element.getOrderType() == OrderType.ASC ? "" : "-") + element.getColumnId()%>">
<%    }
    }%>

<script type="text/javascript">
  function selectColumn(columnId) {
	  opForm.selectedColumnId.value = columnId;
	  opForm.submit();
	  return false;
  }

  function selectCell(columnId, cellId) {
    opForm.selectedColumnId.value = columnId;
    opForm.selectedCellId.value = cellId;
    opForm.submit();
    return false;
  }
</script>



<table class=list cellspacing=0>

    <!-- -------------------------------------- COLUMN WIDTH ------------------------------------------------------- -->
    <% for (Iterator iter = bean.getColumns(); iter.hasNext();) { %>
  <col width="<%=((Column)iter.next()).getWidth()%>%"><% } %>

    <!-- --------------------------------------- COLUMNS HEADERS --------------------------------------------------- -->
  <thead>
    <tr>
    <% // SHOW COLUMNS HEADER
      for (Iterator iter = bean.getColumns(); iter.hasNext();) {
        final Column column = (Column)iter.next(); %>
      <th><%=TableRenderer.renderColumn(column, bean.getOrderType(column.getId()))%></th>
    <%} %>
    </tr>
  </thead>

    <!-- ------------------------------------------- TABLE BODY ---------------------------------------------------- -->

  <tbody>
    <%for (Iterator rows = bean.getRows(); rows.hasNext();) {
        final Row row = (Row)rows.next(); %>
    <tr class=row<%=rowN++&1%>>
    <%
        for (Iterator cols = bean.getColumns(); cols.hasNext();) {
          final Column column = (Column)cols.next();
          final Cell cell = row.getCell(column); %>
        <%=TableRenderer.renderCell(cell, column)%>
    <%
        }%>
    </tr>
    <%}%>
  </tbody>
</table>
<%
  } else {
%>
  Invalid bean type!!! Bean must extends SimpleTableBean
<%
  }
%>
