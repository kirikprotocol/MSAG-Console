<%@ include file="/WEB-INF/inc/code_header.jsp"%><%@ 
page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddExternalAdm,
             ru.novosoft.util.jsp.MultipartServletRequest"%><jsp:useBean id="services_serviceAddExternalAdm" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddExternalAdm" scope="session"/><jsp:setProperty name="services_serviceAddExternalAdm" property="*"/><%

ru.novosoft.smsc.jsp.smsc.services.ServiceAddExternalAdm bean = services_serviceAddExternalAdm;

MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
if (multi != null)
	request = (HttpServletRequest)multi;

if (bean.getStage() == 2 && request.getParameter("jsp") != null)
	FORM_URI = CPATH + request.getParameter("jsp");

bean.setWantAlias(request.getParameter("wantAlias") != null);
bean.setForceDC(request.getParameter("forceDC") != null);

switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
if (bean.getStage() == 1)
{
	FORM_URI = CPATH+"/upload";
	FORM_METHOD = "POST";
	FORM_ENCTYPE = "multipart/form-data";
} else
{
	FORM_URI = CPATH+"/services/serviceAddExternalAdm.jsp";
}

TITLE = "Add service: step " + (bean.getStage() == 0 ? 2 : bean.getStage()+1);
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ 
include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbNext",  "Next",  "Next page");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<%
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
	<table class=properties_list cellspacing=0 cellpadding=0>
	<col width="15%" align=right>
	<col width="85%">
	<tr class=row0>
		<th class=label>Distribute:</th>
		<td><input class=txt type=file name=distribute></td>
	</tr>
	</table>
	<%}
	break;
	case 2:
	{%>
	<h3>hosting parameters:</h3>
	<table class=properties_list cellspacing=0 cellpadding=0>
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
			<td><input class=txt name=port maxlength="5" value="<%=bean.getPortInt() != -1 ? bean.getPort() : ""%>"></td>
		</tr>
		<tr class=rowLast>
			<th>Startup&nbsp;Arguments:</th>
			<td><textarea rows="5" name="startupArgs" wrap="off"><%=bean.getStartupArgs() != null ? StringEncoderDecoder.encode(bean.getStartupArgs()) : ""%></textarea></td>
		</tr>
		</table>
	<%}
	break;
	case 3:
	{
		int rowN = 0;%>
		<table class=properties_list cellspacing=0 cellpadding=0>
		<tr class=row<%=(rowN++)&1%>>
			<th>System Id</th>
			<td><input class=text type="text" name="serviceId" readonly value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>"></td>
		</tr>
		<%@include file="serviceSmeBody.jsp"%>
		</table>
	<%}
	break;
	default:
	{
	}
}%>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbNext",  "Next",  "Next page");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
