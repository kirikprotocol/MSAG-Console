<%@ include file="/sketches/header.jsp"%>
<%@ include file="menu.jsp"%>
<H1><a href="index.jsp">SMSC admin module</a></H1>
<form method="post" action="profiles_update.jsp">
	<input type="Hidden" name="address" value="<%=request.getParameter("address")%>">
	<h2>Profile: <%=request.getParameter("address")%></h2>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list" width="33%">Report mode</th>
			<td class="list">
				<select name="report" style="width: 100%;">
					<option value="0">none</option>
					<option value="1" SELECTED>full</option>
				</select>
			</td>
			</tr>
		<tr class="list">
			<th class="list">CodePage</th>
			<td class="list">
				<select name="codepage" style="width: 100%;">
					<option value="0">Default</option>
					<option value="1" SELECTED>UCS2</option>
				</select>
			</td>
		</tr>
	</table>
	<input type="Submit" value="Apply changes">
</form>
<%@ include file="/sketches/footer.jsp"%>