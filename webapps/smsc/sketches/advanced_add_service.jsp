<%@ include file="header.jsp"%>
<form action="advanced_add_service_2.jsp" method="post">
	<h2>Add service:</h2>
	<h3>hosting parameters:</h3>
	<table class="list">
		<tr class="list">
			<th class="list">Host</th>
			<td class="list">
				<select name="host" style="width: 100%;">
					<option value="esme_dedicated" selected>ESME Dedicated</option>
					<option value="smsc_main">SMSC Main</option>
					<option value="smsc_backup">SMSC Backup</option>
					<option value="mail_gate">mail_gate</option>
					<option value="megatron">megatron</option>
				</select>
			</td>
			<tr class="list">
				<th class="list">Port</th>
				<td class="list"><input name="port" value="1234" style="width: 100%;"></td>
			</tr>
			<tr class="list">
				<th class="list">Distribute</th>
				<td class="list"><input type="file" name="distribute" accept="application/x-zip-compressed" style="width: 100%;"></td>
			</tr>
			<tr class="list" title="maybe, it is not useful">
				<th class="list">Startup Arguments</th>
				<td class="list">
					<textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;">something first
something else</textarea>
				</td>
			</tr>
		</tr>
	</table>
	<div align="right">
		<INPUT type=submit value="Next page">
	</div>
</form>
<%@ include file="footer.jsp"%>