<%@ page isErrorPage="false"%>
<%@ page errorPage="/common/error.jsp"%>
<%@ include file="/common/header_begin.jsp"%>
<%
//AddAdmServiceWizard wizard = (AddAdmServiceWizard) session.getAttribute("AddAdmServiceWizard");
if (wizard == null)
{
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service.jsp"/></head></html><%
} else {

String host = request.getParameter("host") == null ? "" : request.getParameter("host");
int    port = getIntegerParam(request, "port");
String args = request.getParameter("args") == null ? "" : request.getParameter("args");
if (host.length()>0 && port > 0)
{
	wizard.setStage2(host, port, args);
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service_4.jsp"/></head></html><%
} else {
%>
<%@ include file="/common/header_end.jsp"%>
<script>
	function checkPort(field)
	{
		var val = field.value;
		if (val == null || val.length == 0)
		{
			alert("Port cannot be empty");
			field.focus();
			return false;
		}

		var n = parseInt(val);
		if (n == null || n == NaN || isNaN(n) || n.length == 0)
		{
			alert("Port must be integer from 100 to 65535");
			field.focus();
			return false;
		}

		if (n < 100 || n > 65535)
		{
			alert("Port must be in range from 100 to 65535");
			field.focus();
			return false;
		}

		return true;
	}

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
<form onsubmit="return checkPort(port)" method="post">
	<h2>Add service:</h2>
	<h3>hosting parameters:</h3>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list" style="width:25%">Host</th>
			<td class="list">
				<select name="host" style="width: 100%;">
				<%
				for (Iterator i = serviceManager.getHostNames().iterator(); i.hasNext(); )
				{
					String name = StringEncoderDecoder.encode((String) i.next());
					%><option value="<%=name%>"<%=host.equals(name) ? " selected" : ""%>><%=name%></option><%
				}
				%>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Port</th>
			<td class="list"><input type="text" name="port" maxlength="5" style="width: 100%;" onblur="checkPort(this)"></td>
		</tr>
		<tr class="list">
			<th class="list">Startup Arguments</th>
			<td class="list">
				<textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;"></textarea>
			</td>
		</tr>
	</table>
	<div align="right"><input type=submit value="Next page"></div>
</form>
<%@ include file="/common/footer.jsp"%>
<%
}
}%>