<%@ page import="java.util.Iterator,
                 ru.novosoft.smsc.jsp.util.tables.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.table.*"%>

<%
  if (bean instanceof SimpleTableBean) {

%>

<input type=hidden name=column>
<input type=hidden name=selectedRow>
<input type=hidden name=selectedColumn>

<script type="text/javascript">
function selectColumn(column) {
	opForm.column.value = column;
	opForm.submit();
	return false;
}
</script>

<script type="text/javascript">
function setSelectedCell(row, column) {
  opForm.selectedRow.value = row;
  opForm.selectedColumn.value = column;
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
      int colNum = 0;
      for (Iterator iter = bean.getColumns(); iter.hasNext();) {
        final Column column = (Column)iter.next(); %>
      <th><%=TableRenderer.renderColumn(column, colNum)%></th>
      <% colNum++;
      } %>
    </tr>
  </thead>

    <!-- ------------------------------------------- TABLE BODY ---------------------------------------------------- -->

  <tbody>
    <%int rowNum = 0;
      for (Iterator rows = bean.getRows(); rows.hasNext();) {
        final Row row = (Row)rows.next(); %>
    <tr class=row<%=rowN++&1%>>
    <%  colNum = 0;
        for (Iterator cols = bean.getColumns(); cols.hasNext();) {
          final Cell cell = row.getCell((Column)cols.next()); %>
        <%=TableRenderer.renderCell(cell, rowNum, colNum)%>
    <%     colNum++;
        }%>
    </tr>
    <%  rowNum++;
      }%>
  </tbody>
</table>
<%
  } else {
%>
  Invalid bean type!!! Bean must extends SimpleTableBean
<%
  }
%>
