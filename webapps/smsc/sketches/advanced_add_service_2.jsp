<%@ include file="header.jsp"%>
<% String host = request.getParameter("host") == null ? "SMSC Main" : request.getParameter("host"); %>
<% int port = Integer.valueOf(request.getParameter("port") == null ? "0" : request.getParameter("port") ).intValue(); %>
<% String args = request.getParameter("args") == null ? "" : request.getParameter("args"); %>
<% String systemId = "Mail->SMS SME System Id";%>
<%= request.getParameter("distribute") %>
<form action="advanced_add_service_3.jsp" method="post">
	<input type="hidden" name="host" value="<%=URLEncoder.encode(host)%>">
	<input type="hidden" name="port" value="<%=String.valueOf(port)%>">
	<input type="hidden" name="args" value="<%=URLEncoder.encode(args)%>">
	<h2>Add service:</h2>
	<h3>service parameters:</h3>
	<table class="list">
		<tr class="list">
			<th class="list">Type of number</th>
			<td class="list">
				<select name="type of number" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">International</option>
					<option value="2">National</option>
					<option value="3">Network Specific</option>
					<option value="4">Subscriber Number</option>
					<option value="5">Alphanumeric</option>
					<option value="6">Abbreviated</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Numbering plan</th>
			<td class="list">
				<select name="numbering plan" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">ISDN (E163/E164)</option>
					<option value="3">Data (X.121)</option>
					<option value="4">Telex (F.69)</option>
					<option value="6">Land Mobile (E.212)</option>
					<option value="8">National</option>
					<option value="9">Private</option>
					<option value="10">ERMES</option>
					<option value="14">Internet (IP)</option>
					<option value="18">WAP Client Id (to be defined by WAP Forum)</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Interface version</th>
			<td class="list"><input type="text" name="interface version" value="3.4" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Range of Address</th>
			<td class="list"><input type="text" name="range of address" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">System Type</th>
			<td class="list"><input type="text" name="system type" value="" style="width: 100%;"></td>
		</tr>
		<!--tr class="list">
			<th class="list">Password</th>
			<td class="list"><input type="password" name="password" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">retype Password</th>
			<td class="list"><input type="password" name="password" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">another Port</th>
			<td class="list"><input type="text" name="sme port" value="" style="width: 100%;"></td>
		</tr-->
		<tr class="list">
			<th class="list">System Id</th>
			<td class="list"><input type="text" name="system id" value="<%=systemId%>" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">SME NType</th>
			<td class="list"><input type="text" name="sme ntype" value="????" style="width: 100%; color: Red;"></td>
		</tr>
	</table>
	<div align="right">
		<INPUT type=submit value="Next page">
	</div>
</form>
<%@ include file="footer.jsp"%>