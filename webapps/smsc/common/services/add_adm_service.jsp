<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host");%>
<script>
function checkFile(field)
{
	var val = field.value;
	if (val == null || val.length == 0)
	{
		alert("Please, specify service distribute now.");
		field.focus();
		return false;
	}
	return true;
}
</script>
<% if (request.getParameter("error") != null)
{
	%><div style="font: +2; color: Red;">Error: <%=request.getParameter("error")%></div><%
}
%><br>
<form onsubmit="return checkFile(distribute)" method="post" enctype="multipart/form-data" action="<%=urlPrefix%>/upload">
	<input type="hidden" name="jsp" value="<%=urlPrefix+servicesPrefix%>/add_adm_service_2.jsp">
	<%if (request.getParameter("host") != null)
	{
		%><input type="Hidden" name="host" value="<%=request.getParameter("host")%>"><%
	}%>
	<h2>Add service:</h2>
	<h3>hosting parameters:</h3>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list" style="width:25%">Distribute</th>
			<td class="list"><input type="file" name="distribute" style="width: 100%;"></td>
		</tr>
	</table>
	<div align="right">
		<input type=submit name=mbNext value="Next page">
	</div>
</form>
<%@ include file="/common/footer.jsp"%>