<%@ include file="/sketches/header.jsp"%>
<%@ include file="menu.jsp"%>
<h2>Aliases</h2>
<h3>Add alias:</h3>
<form method="post" action="edit_alias_2.jsp">
  <table class="list">
    <thead class="list">
      <tr class="list">
        <th class="list" width="49%">From</th>
        <th class="list" width="49%">To</th>
      </tr>
    </thead>
    <tbody class="list">
      <tr class="list">
        <td class="list"><input type="Text" name="from" value="<%=StringEncoderDecoder.encode(request.getParameter("from") == null ? "" : request.getParameter("from"))%>" style="width: 100%"></td>
        <td class="list"><input type="Text" name="to"   value="<%=StringEncoderDecoder.encode(request.getParameter("to")   == null ? "" : request.getParameter("to"))  %>" style="width: 100%"></td>
      </tr>
    </tbody>
  </table>
  <input type="Hidden" name="<%=request.getParameter("from") != null && request.getParameter("to") != null ? "is_edit" : "is_add"%>" value="true">
  <input type="Submit">
</form>
<%@ include file="/sketches/footer.jsp"%>