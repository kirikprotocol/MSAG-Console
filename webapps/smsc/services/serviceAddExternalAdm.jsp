<%@ include file="/WEB-INF/inc/code_header.jsp"%><%@ 
page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddExternalAdm,
             ru.novosoft.util.jsp.MultipartServletRequest"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddExternalAdm" scope="session"/><jsp:setProperty name="bean" property="*"/><%

if (bean.getStage() < 2)
{
FORM_URI = CPATH+"/upload";
FORM_METHOD = "POST\" enctype=\"multipart/form-data";
}
MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
if (multi != null)
	request = (HttpServletRequest)multi;

if (bean.getStage() == 2 && request.getParameter("jsp") != null)
	FORM_URI = CPATH + request.getParameter("jsp");

switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case ServiceAddExternalAdm.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ServiceAddExternalAdm.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ServiceAddExternalAdm.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}

if (bean.getStage() == 1 && multi != null)
{
	bean.receiveFile((MultipartServletRequest)request.getAttribute("multipart.request"));
}
TITLE = "Services List / Add service: step " + (bean.getStage() == 0 ? 2 : bean.getStage()+1);
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ 
include file="/WEB-INF/inc/html_3_header.jsp"%><%@ 
include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%@ include file="/WEB-INF/inc/messages.jsp"%><%
if (bean.getHostName() != null && bean.getStage() != 2)
{
%><input type=hidden name=hostName value="<%=bean.getHostName()%>"><%
}
%><input type=hidden name=stage value="<%=bean.getStage()%>"><%

switch (bean.getStage())
{
	case 0:
	case 1:
	{%>
	<input type="hidden" name="jsp" value="/services/serviceAddExternalAdm.jsp">
	<div class=secInfo>Select service distribute:</div>
	<table class=secRep cellspacing=1 width="100%">
	<col width="15%" align=right>
	<col width="85%">
	<tr class=rowLast>
		<th class=label>Distribute:</th>
		<td><input class=txtW type=file name=distribute></td>
	</tr>
	</table>
	<%}
	break;
	case 2:
	{%>
	<h3>hosting parameters:</h3>
	<table class=frm0 cellspacing=0 width="100%">
	<col width="15%" align=right>
	<col width="85%">
	<tr class=row0>
		<th>host name:</th>
		<td><select name="hostName">
				<%
				for (Iterator i = bean.getHostNames().iterator(); i.hasNext(); )
				{
					String name = (String) i.next();
					String encodedName = StringEncoderDecoder.encode(name);
					%><option value="<%=encodedName%>"<%=name.equals(bean.getHostName()) ? " selected" : ""%>><%=name%></option><%
				}
				%>
				</select></td>
		</tr>
		<tr class=row0>
			<th>host port:</th>
			<td><input class=txt name=port maxlength="5" value="<%=bean.getPort() != -1 ? (""+bean.getPort()) : ""%>"></td>
		</tr>
		<tr class=rowLast>
			<th>Startup&nbsp;Arguments:</th>
			<td><textarea class=txtW rows="5" name="startupArgs" wrap="off"><%=bean.getStartupArgs() != null ? StringEncoderDecoder.encode(bean.getStartupArgs()) : ""%></textarea></td>
		</tr>
		</table>
	<%}
	break;
	case 3:
	{%>


<table class=frm0 cellspacing=0 width="100%">
<tr class=row0>
	<th>System Id</th>
	<td><input type="text" name="serviceId" readonly value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>"></td>
</tr>
<tr class=row0>
	<th>System Type</th>
	<td><input type="text" name="systemType" value="<%=bean.getSystemType() != null ? StringEncoderDecoder.encode(bean.getSystemType()) : ""%>" maxlength="13"></td>
</tr>
<tr class=row0>
	<th>Type of number</th>
	<td>
		<select name="typeOfNumber">
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
<tr class=row0>
	<th>Numbering plan</th>
	<td>
		<select name="numberingPlan">
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
<tr class=row0>
	<th>Interface version</th>
	<td><input type="text" name="interfaceVersion" value="3.4" readonly></td>
</tr>
<tr class=row0>
	<th>Range of Address</th>
	<td><input maxlength="41" type="text" name="rangeOfAddress" value="<%=bean.getRangeOfAddress() != null ? StringEncoderDecoder.encode(bean.getRangeOfAddress()) : ""%>"></td>
</tr>
<tr class=rowLast>
	<th>Password</th>
	<td><input maxlength="41" type="text" name="password" value="<%=bean.getPassword() != null ? StringEncoderDecoder.encode(bean.getPassword()) : ""%>"></td>
</tr>
</table>
	<%}
	break;
	default:
	{
	}
}%>

<div class=secButtons>
<input class=btn type=submit name=mbNext value="Next" title="Next page">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
