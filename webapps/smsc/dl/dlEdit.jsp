<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="dl_dlEdit_bean" class="ru.novosoft.smsc.jsp.dl.dlEdit" scope="session"/>
<jsp:setProperty name="dl_dlEdit_bean" property="*"/>
<%
ru.novosoft.smsc.jsp.dl.dlEdit bean = dl_dlEdit_bean;
TITLE = "Edit distribution list";
MENU0_SELECTION = "MENU0_DL";
final boolean isEditing = true;

switch (bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case dlEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case dlEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case dlEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="dlBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save distribution list">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>