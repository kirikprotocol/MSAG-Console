<%@ page isErrorPage="false"%>
<%@ page errorPage="/common/error.jsp"%>
<%@ include file="/common/header_begin.jsp"%>
<%
//AddAdmServiceWizard wizard = (AddAdmServiceWizard) session.getAttribute("AddAdmServiceWizard");
if (wizard == null)
{
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service.jsp"/></head></html><%
} else {

	String systemId = wizard.getSystemId();

	String systemType = request.getParameter("system_type") == null ? "" : request.getParameter("system_type");
	int    typeOfNumber     = getIntegerParam(request, "type of number");
	int    numberingPlan    = getIntegerParam(request, "numbering plan");
	int    interfaceVersion = 0x34;//getIntegerParam(request, "interface version");
	String rangeOfAddress   = request.getParameter("range of address") == null ? "" : request.getParameter("range of address");

	if (systemType.length() > 0
		&& typeOfNumber     != Integer.MIN_VALUE
		&& numberingPlan    != Integer.MIN_VALUE
		&& interfaceVersion != Integer.MIN_VALUE
		&& rangeOfAddress.length() > 0)
	{
		wizard.setStage3(systemType, typeOfNumber, numberingPlan, interfaceVersion, rangeOfAddress);
		%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service_5.jsp"/></head><body>Please, be patient...</body></html><%
	} else {
		%>
		<%@ include file="/common/header_end.jsp"%>
		<script>
		function checkSystemType(field)
		{
			if (field.value.length < 1)
			{
				alert("Please, enter system type");
				field.focus();
				return false;
			}
			return true;
		}
		</script>
		<form onsubmit="return checkSystemType(system_type)" method="post">
		  <h2>Add service:</h2>
		  <h3>service parameters:</h3>
		  <table class="list" cellspacing="0">
				<tr class="list">
					<th class="list">System Id</th>
					<td class="list"><input type="text" name="system id" value="<%=systemId%>" readonly style="width: 100%;"></td>
				</tr>
				<tr class="list">
					<th class="list">System Type</th>
					<td class="list"><input type="text" name="system_type" maxlength="13" style="width: 100%;" onblur="checkSystemType(this)"></td>
				</tr>
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
					<td class="list"><input maxlength="41" type="text" name="range of address" value="" style="width: 100%;"></td>
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
				</tr>
				<tr class="list">
					<th class="list">SME NType</th>
					<td class="list"><input type="text" name="sme ntype" value="????" style="width: 100%; color: Red;"></td>
				</tr-->
			</table>
			<div align="right">
				<INPUT type=submit value="Next page">
			</div>
		</form>
		<%@ include file="/common/footer.jsp"%>
<%	}
}%>