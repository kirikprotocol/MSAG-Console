<%@ include file="/sketches/header.jsp"%>
<%@ include file="menu.jsp"%>
<H1><a href="index.jsp">SMSC admin module</a></H1>
<h2>Profile: <%=request.getParameter("address")%></h2>
Changes applied
<table class="list" cellspacing="0">
	<tr class="list">
		<th class="list" width="33%">Report mode</th>
		<td class="list"><%=request.getParameter("report") == null || request.getParameter("report").equals("0") ? "none" : "full"%></td>
		</tr>
	<tr class="list">
		<th class="list">CodePage</th>
		<td class="list"><%=request.getParameter("codepage") == null || request.getParameter("codepage").equals("0") ? "Default" : "UCS2"%></td>
	</tr>
</table>
<%@ include file="/sketches/footer.jsp"%>