<%@ include file="/header.jsp"%>
<form action="add_host_2.jsp">
	<h2>Add host:</h2>
	<table border="1" class="list" cellspacing="0">
		<tr class="list">
			<th class="list">host name</th>
			<td class="list"><input type="text" name="host" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Port</th>
			<td class="list"><input name="port" value="" style="width: 100%;"></td>
		</tr>
	</table>
	<input type=submit value="add host" name="submit">
</form>
<%@ include file="/common/footer.jsp"%>