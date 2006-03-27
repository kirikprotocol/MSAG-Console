<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.closedgroups.ClosedGroupEdit" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.closedgroups.ClosedGroupEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("closedgroups.editSubTitle");
    switch (bean.process(request)) {
        case ClosedGroupEdit.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case ClosedGroupEdit.RESULT_OK:

            break;
        case ClosedGroupEdit.RESULT_ERROR:

            break;
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_PROVIDERS";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "closedgroups.saveEditHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "closedgroups.cancelEditHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<input type=hidden name="id" value="<%=bean.getId()%>">
<%@ include file="closedGroupBody.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "closedgroups.saveEditHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "closedgroups.cancelEditHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>