<%@ include file="/common/header_begin.jsp"%>
<%@ include file="utils.jsp"%>
</head>
<body>
<form method="post" action="create_subject_2.jsp">
  <h4>Add Subject</h4>
  <table class="list" cellspacing="0">
    <tr class="list">
      <th class="list" width="1%" nowrap>Subject&nbsp;name</th>
      <td class="list" width="99%"><input type="text" name="name"></td>
    </tr>
    <tr class="list">
      <th class="list" width="1%" nowrap>Default&nbsp;SME</th>
      <td class="list" width="99%"><%=show_set_combo("sme", serviceManager.getServiceIds())%></td>
    </tr>
    <tr class="list">
      <th class="list" width="1%" nowrap>Masks</th>
      <td class="list" width="99%"><textarea name="masks" style="WIDTH: 100%" rows=5></textarea></td>
    </tr>
  </table>
  <input type="Submit">
</form>
</body>
</html>