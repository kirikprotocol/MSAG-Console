<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%
boolean isEdit = request.getParameter("Address") != null && request.getParameter("Alias") != null;
String address = isEdit ? StringEncoderDecoder.encode(request.getParameter("Address")) : "";
String alias   = isEdit ? StringEncoderDecoder.encode(request.getParameter("Alias"  )) : "";
%>
<h4>Aliases</h4>
<h5><%=isEdit ? "Edit" : "Add"%> alias:</h5>
<form method="post" action="edit_2.jsp">
  <table class="list">
    <thead class="list">
      <tr class="list">
        <th class="list" width="49%">Alias</th>
        <th class="list" width="49%">Address</th>
      </tr>
    </thead>
    <tbody class="list">
      <tr class="list">
        <td class="list"><input type="Text" name="alias"   value="<%=alias  %>" style="width: 100%"></td>
        <td class="list"><input type="Text" name="address" value="<%=address%>" style="width: 100%"></td>
      </tr>
    </tbody>
  </table>
  <input type="Hidden" name="old_address" value="<%=address%>">
  <input type="Hidden" name="old_alias"   value="<%=alias  %>">
  <input type="Hidden" name="<%=isEdit ? "is_edit" : "is_add"%>" value="true">
  <input type="Submit">
</form>
<%@ include file="/sketches/footer.jsp"%>