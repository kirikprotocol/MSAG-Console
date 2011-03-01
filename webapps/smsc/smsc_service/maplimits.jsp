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

    param(out, "maplimits.dlglimit.in", "dlglimit.in", bean.getIntParam("dlglimit.in"));
    param(out, "maplimits.dlglimit.insri", "dlglimit.insri", bean.getIntParam("dlglimit.insri"));
    param(out, "maplimits.dlglimit.ussd", "dlglimit.ussd", bean.getIntParam("dlglimit.ussd"));
    param(out, "maplimits.dlglimit.outsri", "dlglimit.outsri", bean.getIntParam("dlglimit.outsri"));
    param(out, "maplimits.dlglimit.niussd", "dlglimit.niussd", bean.getIntParam("dlglimit.niussd"));

    finishParams(out);
    finishSection(out);

    //################################## ussd #############################
    startSection(out, "ussd", "maplimits.ussd", true);
    startParams(out);

    param(out, "maplimits.ussd.no_sri_codes", "ussd.no_sri_codes", bean.getStringParam("ussd.no_sri_codes"));
    param(out, "maplimits.ussd.cond_sri_codes", "ussd.cond_sri_codes", bean.getStringParam("ussd.cond_sri_codes"));
    param(out, "maplimits.ussd.ati_codes", "ussd.ati_codes", bean.getStringParam("ussd.ati_codes"));
    param(out, "maplimits.ussd.openRespRealAddr", "ussd.openRespRealAddr", bean.getStringParam("ussd.openRespRealAddr"));

    finishParams(out);
    finishSection(out);

    //################################## clevels #############################
    startSection(out, "clevels", "maplimits.clevels", true);
    for( int ii = 1; ii <= 8; ii++ ) {
      startSection(out, "clevels.level"+ii, "maplimits.clevels.level"+ii, true);
      startParams(out);
      param(out, "maplimits.clevels.level.dialogsLimit", "clevels.level"+ii+".dialogsLimit", bean.getIntParam("clevels.level"+ii+".dialogsLimit"));
      param(out, "maplimits.clevels.level.failUpperLimit", "clevels.level"+ii+".failUpperLimit", bean.getIntParam("clevels.level"+ii+".failUpperLimit"));
      param(out, "maplimits.clevels.level.failLowerLimit", "clevels.level"+ii+".failLowerLimit", bean.getIntParam("clevels.level"+ii+".failLowerLimit"));
      param(out, "maplimits.clevels.level.okToLower", "clevels.level"+ii+".okToLower", bean.getIntParam("clevels.level"+ii+".okToLower"));
      finishParams(out);
      finishSection(out);
    }
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
