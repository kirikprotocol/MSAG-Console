<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Subjects</h4>
<%
SubjectList subjects = routeManager.getSubjects();
%><table class="list" cellspacing="0">
  <thead class="list">
    <tr class="list">
      <th class="list">Name</th>
      <th class="list">Default&nbsp;SME</th>
      <th class="list">Masks</th>
      <th class="list"><a href="edit.jsp">Add</a></th>
    </tr>
  </thead>
  <tbody><%
for (Iterator i = subjects.iterator(); i.hasNext(); )
{
  Subject s = (Subject) i.next();
  %><tr class="list">
    <td class="list"><%=StringEncoderDecoder.encode(s.getName())%></td>
    <td class="list"><%=StringEncoderDecoder.encode(s.getDefaultSme().getId())%></td>
    <td class="list"><%
    MaskList masks = s.getMasks();
    for (Iterator j = masks.iterator(); j.hasNext();)
    {
      Mask m = (Mask) j.next();
      %><%=StringEncoderDecoder.encode(m.getMask()) + (j.hasNext() ? "<br>" : "")%><%
    }
    %></td>
    <td class="list"><a href="edit.jsp?name=<%=URLEncoder.encode(s.getName())%>">Edit</a>&nbsp;<a href="delete.jsp?name=<%=URLEncoder.encode(s.getName())%>">Delete</a></td>
  </tr><%
}

%></tbody>
</table><%@ include file="/common/footer.jsp"%>