<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Aliases</h4>

<table class="list">
  <thead class="list">
    <tr class="list">
      <th class="list" width="49%">Alias</th>
      <th class="list" width="49%">Address</th>
      <th width="2%" class="list"><a href="edit.jsp">Add</a></th>
    </tr>
  </thead>
  <tbody class="list">
    <% 
    for (Iterator i = smsc.getAliases().iterator(); i.hasNext(); )
    {
      Alias a = (Alias) i.next();
      %>
      <tr class="list">
        <td class="list"><%=a.getAlias().getMask()%></td>
        <td class="list"><%=a.getAddress().getMask()%></td>
        <td class="list"><a href="edit.jsp?alias=<%=URLEncoder.encode(a.getAlias().getMask())%>&address=<%=URLEncoder.encode(a.getAddress().getMask())%>">Edit</a>&nbsp;<a href="delete.jsp?alias=<%=URLEncoder.encode(a.getAlias().getMask())%>&address=<%=URLEncoder.encode(a.getAddress().getMask())%>">Delete</a></td>
      </tr>
    <%}%>
  </tbody>
</table>
<%@ include file="/common/footer.jsp"%>