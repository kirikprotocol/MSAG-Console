<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesAdd" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("profiles.addTitle");
    switch (bean.process(request)) {
        case ProfilesAdd.RESULT_DONE:
            if ("lookup".equals(bean.getReturnPath()))
                response.sendRedirect("lookup.jsp?profile=" + URLEncoder.encode(bean.getMask()));
            else if ("groups".equals(bean.getReturnPath()))
                response.sendRedirect("groups.jsp");
            else if ("profiles".equals(bean.getReturnPath()))
                response.sendRedirect("index.jsp");
            else
                response.sendRedirect("index.jsp");
            return;
        case ProfilesAdd.RESULT_OK:
            break;
        case ProfilesAdd.RESULT_ERROR:
            break;
    }
    boolean isEdit = false;
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_PROFILES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "profiles.add", "profiles.addHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "profiles.cancelAddHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="profileBody.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "profiles.add", "profiles.addHint");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "profiles.cancelAddHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>