<%@ include file="/common/header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.service.*"%>
<%
	String systemId         = request.getParameter("systemId") == null ? "" : request.getParameter("systemId");
	String systemType       = request.getParameter("system_type") == null ? "" : request.getParameter("system_type");
	int    typeOfNumber     = getIntegerParam(request, "type of number");
	int    numberingPlan    = getIntegerParam(request, "numbering plan");
	int    interfaceVersion = 0x34;//getIntegerParam(request, "interface version");
	String password         = request.getParameter("password") == null ? "" : request.getParameter("password");
	String rangeOfAddress   = request.getParameter("range of address") == null ? "" : request.getParameter("range of address");
	
	if (systemType.length() > 0 
		&& typeOfNumber     != Integer.MIN_VALUE
		&& numberingPlan    != Integer.MIN_VALUE
		&& interfaceVersion != Integer.MIN_VALUE
		&& rangeOfAddress.length() > 0)
	{
		session.setAttribute("AddNonAdmServiceWizard", new AddNonAdmServiceWizard(systemId, systemType, typeOfNumber, numberingPlan, interfaceVersion, password, rangeOfAddress));
		%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_non_service_2.jsp"/></head><body>Please, be patient...</body></html><%
	} else {
%>
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
			<td class="list"><input type="text" name="systemId" value="<%=StringEncoderDecoder.encode(systemId)%>" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">System Type</th>
			<td class="list"><input type="text" name="system_type" value="<%=StringEncoderDecoder.encode(systemType)%>" maxlength="13" style="width: 100%;" onblur="checkSystemType(this)"></td>
		</tr>
		<tr class="list">
			<th class="list">Type of number</th>
			<td class="list">
				<select name="type of number" style="width: 100%;">
					<option value="0"<%=typeOfNumber == 0 ? " selected" : ""%>>Unknown</option>
					<option value="1"<%=typeOfNumber == 1 ? " selected" : ""%>>International</option>
					<option value="2"<%=typeOfNumber == 2 ? " selected" : ""%>>National</option>
					<option value="3"<%=typeOfNumber == 3 ? " selected" : ""%>>Network Specific</option>
					<option value="4"<%=typeOfNumber == 4 ? " selected" : ""%>>Subscriber Number</option>
					<option value="5"<%=typeOfNumber == 5 ? " selected" : ""%>>Alphanumeric</option>
					<option value="6"<%=typeOfNumber == 6 ? " selected" : ""%>>Abbreviated</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Numbering plan</th>
			<td class="list">
				<select name="numbering plan" style="width: 100%;">
					<option value="0" <%=numberingPlan == 0  ? " selected" : ""%>>Unknown</option>
					<option value="1" <%=numberingPlan == 1  ? " selected" : ""%>>ISDN (E163/E164)</option>
					<option value="3" <%=numberingPlan == 3  ? " selected" : ""%>>Data (X.121)</option>
					<option value="4" <%=numberingPlan == 4  ? " selected" : ""%>>Telex (F.69)</option>
					<option value="6" <%=numberingPlan == 6  ? " selected" : ""%>>Land Mobile (E.212)</option>
					<option value="8" <%=numberingPlan == 8  ? " selected" : ""%>>National</option>
					<option value="9" <%=numberingPlan == 9  ? " selected" : ""%>>Private</option>
					<option value="10"<%=numberingPlan == 10 ? " selected" : ""%>>ERMES</option>
					<option value="14"<%=numberingPlan == 14 ? " selected" : ""%>>Internet (IP)</option>
					<option value="18"<%=numberingPlan == 18 ? " selected" : ""%>>WAP Client Id (to be defined by WAP Forum)</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Interface version</th>
			<td class="list"><input type="text" name="interface version" value="3.4" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Range of Address</th>
			<td class="list"><input maxlength="41" type="text" name="range of address" value="<%=StringEncoderDecoder.encode(rangeOfAddress)%>" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Password</th>
			<td class="list"><input maxlength="41" type="text" name="password" value="<%=StringEncoderDecoder.encode(password)%>" style="width: 100%;"></td>
		</tr>
	</table>
	<div align="right">
		<INPUT type=submit value="Add SME">
	</div>
</form>
<%@ include file="/common/footer.jsp"%>
<%}%>