<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.MapLimits"/>
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE = getLocString("maplimits.configuration");
  MENU0_SELECTION = "MENU0_SMSC_MAP_LIMITS";
  bean.process(request);
%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>

<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset");
    page_menu_space(out);
    page_menu_end(out);
%>

<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<%
    //################################## admin #############################
    startSection(out, "dlglimit", "maplimits.dlglimit", true);
    startParams(out);

    param(out, "in", "dlglimit.in", bean.getIntParam("dlglimit.in"));
    param(out, "insri", "dlglimit.insri", bean.getIntParam("dlglimit.insri"));
    param(out, "ussd", "dlglimit.ussd", bean.getIntParam("dlglimit.ussd"));
    param(out, "out", "dlglimit.out", bean.getIntParam("dlglimit.out"));
    param(out, "niussd", "dlglimit.niussd", bean.getIntParam("dlglimit.niussd"));

    finishParams(out);
    finishSection(out);
%>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
