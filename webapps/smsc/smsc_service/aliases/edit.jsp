<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%
boolean isEdit = request.getParameter("Address") != null && request.getParameter("Alias") != null;
String address = isEdit ? StringEncoderDecoder.encode(request.getParameter("Address")) : "";
String alias   = isEdit ? StringEncoderDecoder.encode(request.getParameter("Alias"  )) : "";
boolean hide   = isEdit ? (request.getParameter("Hide") != null && request.getParameter("Hide").equalsIgnoreCase("true")) : false;
%>
<h4>Aliases</h4>
<h5><%=isEdit ? "Edit" : "Add"%> alias:</h5>
<form method="post" action="edit_2.jsp">
  <table class="list">
    <thead class="list">
      <tr class="list">
        <th class="list" width="49%">Alias</th>
        <th class="list" width="49%">Address</th>
		<th class="list" width="1%">Hide</th>
      </tr>
    </thead>
    <tbody class="list">
      <tr class="list">
        <td class="list"><input type="Text" name="alias"   value="<%=alias  %>" style="width: 100%"></td>
        <td class="list"><input type="Text" name="address" value="<%=address%>" style="width: 100%"></td>
		<td class="list"><input type="Checkbox" name="hide" <%=hide ? "checked" : ""%>/></td>
      </tr>
    </tbody>
  </table>
  <input type="Hidden" name="old_address" value="<%=address%>">
  <input type="Hidden" name="old_alias"   value="<%=alias  %>">
  <input type="Hidden" name="old_hide"    value="<%=hide ? "true" : "false"%>">
  <input type="Submit">
</form>
<%@ include file="/common/footer.jsp"%>