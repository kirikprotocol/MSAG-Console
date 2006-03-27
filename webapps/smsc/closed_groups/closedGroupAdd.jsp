<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.closedgroups.ClosedGroupAdd" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.closedgroups.ClosedGroupAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("closedgroups.addTitle");
    switch (bean.process(request)) {
        case ClosedGroupAdd.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case ClosedGroupAdd.RESULT_OK:

            break;
        case ClosedGroupAdd.RESULT_ERROR:

            break;
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_CLOSEDGROUPS";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "closedgroups.saveAddHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "closedgroups.cancelAddHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="closedGroupBody.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "closedgroups.saveAddHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "closedgroups.cancelAddHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>