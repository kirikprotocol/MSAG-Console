<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.proxysme.Index,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.proxysme.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE="Proxy SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";
	//FORM_METHOD = "GET";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
/*		case Index.RESULT_DONE:
			response.sendRedirect("index.jsp");
			return;
*/		case Index.RESULT_DONE:
		case Index.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Index.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<% int row = 0;%>
<div class=page_subtitle>Options</div>
<table class=properties_list cellspacing=0>
<col width="10%">
<tr class=row<%=row++&1%>>
	<th>admin host</th>
	<td><input class=txt name="adminHost" value="<%=bean.getAdminHost()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>admin port</th>
	<td><input class=txt name="adminPort" value="<%=bean.getAdminPort()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>queue length</th>
	<td><input class=txt name="queueLength" value="<%=bean.getQueueLength()%>"></td>
</tr>
</table>
<%row = 0;%>
<div class=page_subtitle>Left SME</div>
<table class=properties_list cellspacing=0>
<col width="10%">
<tr class=row<%=row++&1%>>
	<th>host</th>
	<td><input class=txt name="left_host" value="<%=bean.getLeft_host()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>port</th>
	<td><input class=txt name="left_port" value="<%=bean.getLeft_port()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>sid</th>
	<td><input class=txt name="left_sid" value="<%=bean.getLeft_sid()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>password</th>
	<td><input class=txt name="left_password" value="<%=bean.getLeft_password()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>origAddr</th>
	<td><input class=txt name="left_origAddr" value="<%=bean.getLeft_origAddr()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>systemType</th>
	<td><input class=txt name="left_systemType" value="<%=bean.getLeft_systemType()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>timeOut</th>
	<td><input class=txt name="left_timeOut" value="<%=bean.getLeft_timeOut()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>defaultDCS</th>
	<td>
	  <select class=txt name="left_defaultDCS">
	    <option value="0" <%=bean.getLeft_defaultDCS()==0?"SELECTED":""%>>GSM default alphabet</option>
	    <option value="0" <%=bean.getLeft_defaultDCS()==3?"SELECTED":""%>>Latin1 (ISO8859-1)</option>
	  </select>
	</td>
</tr>
</table>
<%row = 0;%>
<div class=page_subtitle>Right SME</div>
<table class=properties_list cellspacing=0>
<col width="10%">
<tr class=row<%=row++&1%>>
	<th>host</th>
	<td><input class=txt name="right_host" value="<%=bean.getRight_host()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>port</th>
	<td><input class=txt name="right_port" value="<%=bean.getRight_port()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>sid</th>
	<td><input class=txt name="right_sid" value="<%=bean.getRight_sid()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>password</th>
	<td><input class=txt name="right_password" value="<%=bean.getRight_password()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>origAddr</th>
	<td><input class=txt name="right_origAddr" value="<%=bean.getRight_origAddr()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>systemType</th>
	<td><input class=txt name="right_systemType" value="<%=bean.getRight_systemType()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>timeOut</th>
	<td><input class=txt name="right_timeOut" value="<%=bean.getRight_timeOut()%>"></td>
</tr>
<tr class=row<%=row++&1%>>
	<th>defaultDCS</th>
	<td>
	  <select class=txt name="right_defaultDCS">
	    <option value="0" <%=bean.getRight_defaultDCS()==0?"SELECTED":""%>>GSM default alphabet</option>
	    <option value="0" <%=bean.getRight_defaultDCS()==3?"SELECTED":""%>>Latin1 (ISO8859-1)</option>
	  </select>
	</td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply", "Apply", "Apply changes");
page_menu_button(out, "mbClear", "Clear", "Cancel changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>