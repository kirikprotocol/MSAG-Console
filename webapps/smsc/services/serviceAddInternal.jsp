<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case ServiceAddInternal.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ServiceAddInternal.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ServiceAddInternal.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%@ include file="/WEB-INF/inc/messages.jsp"%>


<h1>Add service: step 2</h1>
Select service type:<br>

<table cellspacing="0" border=1>
<tr>
	<th>System Id</th>
	<td><input type="text" name="serviceId" value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>" style="width: 100%;"></td>
</tr>
<tr>
	<th>System Type</th>
	<td><input type="text" name="systemType" value="<%=StringEncoderDecoder.encode(bean.getSystemType())%>" maxlength="13" style="width: 100%;" onblur="checkSystemType(this)"></td>
</tr>
<tr>
	<th>Type of number</th>
	<td>
		<select name="typeOfNumber" style="width: 100%;">
		<option value="0"<%=bean.getTypeOfNumber() == 0 ? " selected" : ""%>>Unknown</option>
		<option value="1"<%=bean.getTypeOfNumber() == 1 ? " selected" : ""%>>International</option>
		<option value="2"<%=bean.getTypeOfNumber() == 2 ? " selected" : ""%>>National</option>
		<option value="3"<%=bean.getTypeOfNumber() == 3 ? " selected" : ""%>>Network Specific</option>
		<option value="4"<%=bean.getTypeOfNumber() == 4 ? " selected" : ""%>>Subscriber Number</option>
		<option value="5"<%=bean.getTypeOfNumber() == 5 ? " selected" : ""%>>Alphanumeric</option>
		<option value="6"<%=bean.getTypeOfNumber() == 6 ? " selected" : ""%>>Abbreviated</option>
		</select>
	</td>
</tr>
<tr>
	<th>Numbering plan</th>
	<td>
		<select name="numberingPlan" style="width: 100%;">
		<option value="0" <%=bean.getNumberingPlan() == 0  ? " selected" : ""%>>Unknown</option>
		<option value="1" <%=bean.getNumberingPlan() == 1  ? " selected" : ""%>>ISDN (E163/E164)</option>
		<option value="3" <%=bean.getNumberingPlan() == 3  ? " selected" : ""%>>Data (X.121)</option>
		<option value="4" <%=bean.getNumberingPlan() == 4  ? " selected" : ""%>>Telex (F.69)</option>
		<option value="6" <%=bean.getNumberingPlan() == 6  ? " selected" : ""%>>Land Mobile (E.212)</option>
		<option value="8" <%=bean.getNumberingPlan() == 8  ? " selected" : ""%>>National</option>
		<option value="9" <%=bean.getNumberingPlan() == 9  ? " selected" : ""%>>Private</option>
		<option value="10"<%=bean.getNumberingPlan() == 10 ? " selected" : ""%>>ERMES</option>
		<option value="14"<%=bean.getNumberingPlan() == 14 ? " selected" : ""%>>Internet (IP)</option>
		<option value="18"<%=bean.getNumberingPlan() == 18 ? " selected" : ""%>>WAP Client Id (to be defined by WAP Forum)</option>
		</select>
	</td>
</tr>
<tr>
	<th>Interface version</th>
	<td><input type="text" name="interfaceVersion" value="3.4" readonly style="width: 100%;"></td>
</tr>
<tr>
	<th>Range of Address</th>
	<td><input maxlength="41" type="text" name="rangeOfAddress" value="<%=StringEncoderDecoder.encode(bean.getRangeOfAddress())%>" style="width: 100%;"></td>
</tr>
<tr>
	<th>Password</th>
	<td><input maxlength="41" type="text" name="password" value="<%=StringEncoderDecoder.encode(bean.getPassword())%>" style="width: 100%;"></td>
</tr>
</table>

<%if (bean.getHostName() != null)
{
%><input type=hidden name=hostName value="<%=bean.getHostName()%>"><%
}%>
<div class=but0>
<input class=btn type=submit name=mbNext value="Finish" title="Add service">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
