<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("subjects.editTitle");
int beanResult = bean.process(request);
switch(beanResult)
{
	case SubjectsEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SubjectsEdit.RESULT_OK:

		break;
	case SubjectsEdit.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.applyChanges");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%boolean isNew = false;%>
<%@ include file="subjectBody.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.applyChanges");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>