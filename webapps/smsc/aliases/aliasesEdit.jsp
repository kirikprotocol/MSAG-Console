<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit alias";
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case AliasesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case AliasesEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case AliasesEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type=hidden name=oldAlias value="<%=bean.getOldAlias()%>">
<input type=hidden name=oldAddress value="<%=bean.getOldAddress()%>">
<input type=hidden name=oldHide value=<%=bean.isOldHide()%>>
<%@ include file="aliasBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save alias">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>