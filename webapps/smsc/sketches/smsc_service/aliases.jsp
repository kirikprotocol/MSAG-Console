<%@ include file="/sketches/header.jsp"%>
<TABLE>
	<TR>
		<TD><A href="../routes/index.jsp"><h2>Routes</h2></A></TD>
		<TD><A href="aliases.jsp"><h2>Aliases</h2></A></TD>
	</TR>
</TABLE>
<h2>Aliases</h2>
<table class="list">
  <thead class="list">
    <tr class="list">
      <th class="list" width="49%">From</th>
      <th class="list" width="49%">To</th>
      <th width="2%" class="list"><a href="edit_alias.jsp">Add</a></th>
    </tr>
  </thead>
  <tbody class="list">
    <tr class="list">
      <td class="list">50????</td>
      <td class="list">+7902915????</td>
      <td class="list"><a href="edit_alias.jsp?from=<%=URLEncoder.encode("50????")%>&to=<%=URLEncoder.encode("+7902915????")%>">Edit</a>&nbsp;<a href="delete_alias.jsp?from=<%=URLEncoder.encode("50????")%>&to=<%=URLEncoder.encode("+7902915????")%>">Delete</a></td>
    </tr>
    <tr class="list">
      <td class="list">14????</td>
      <td class="list">+7902914????</td>
      <td class="list"><a href="edit_alias.jsp?from=<%=URLEncoder.encode("14????")%>&to=<%=URLEncoder.encode("+7902914????")%>">Edit</a>&nbsp;<a href="delete_alias.jsp?from=<%=URLEncoder.encode("14????")%>&to=<%=URLEncoder.encode("+7902914????")%>">Delete</a></td>
    </tr>
    <tr class="list">
      <td class="list">13????</td>
      <td class="list">+7902913????</td>
      <td class="list"><a href="edit_alias.jsp?from=<%=URLEncoder.encode("13????")%>&to=<%=URLEncoder.encode("+7902913????")%>">Edit</a>&nbsp;<a href="delete_alias.jsp?from=<%=URLEncoder.encode("13????")%>&to=<%=URLEncoder.encode("+7902913????")%>">Delete</a></td>
    </tr>
    <tr class="list">
      <td class="list">??????</td>
      <td class="list">+73832?????</td>
      <td class="list"><a href="edit_alias.jsp?from=<%=URLEncoder.encode("??????")%>&to=<%=URLEncoder.encode("+73832??????")%>">Edit</a>&nbsp;<a href="delete_alias.jsp?from=<%=URLEncoder.encode("??????")%>&to=<%=URLEncoder.encode("+73832?????")%>">Delete</a></td>
    </tr>
    <tr class="list">
      <td class="list">911</td>
      <td class="list">01</td>
      <td class="list"><a href="edit_alias.jsp?from=<%=URLEncoder.encode("911")%>&to=<%=URLEncoder.encode("01")%>">Edit</a>&nbsp;<a href="delete_alias.jsp?from=<%=URLEncoder.encode("911")%>&to=<%=URLEncoder.encode("01")%>">Delete</a></td>
    </tr>
  </tbody>
</table>
<%@ include file="/sketches/footer.jsp"%>