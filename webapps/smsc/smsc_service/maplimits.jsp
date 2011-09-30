<%@ page import="java.util.*" %>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.Validation" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.smsc_service.MapLimits" %>
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

  <input type="hidden" id="init" name="init" value="true"/>
<%
    //################################## admin #############################
    startSection(out, "dlglimit", "maplimits.dlglimit", true);
    startParams(out);

//  param(out, label, id, value, null, null, false, validation);
    param(out, "maplimits.dlglimit.in", "dlglimitIn", bean.getDlglimitIn());
    param(out, "maplimits.dlglimit.insri", "dlglimitInsri", bean.getDlglimitInsri());
    param(out, "maplimits.dlglimit.ussd", "dlglimitUssd", bean.getDlglimitUssd());
    param(out, "maplimits.dlglimit.outsri", "dlglimitOutSri", bean.getDlglimitOutSri());
    param(out, "maplimits.dlglimit.niussd", "dlglimitNiussd", bean.getDlglimitNiussd());

    finishParams(out);
    finishSection(out);

    //################################## ussd #############################
    startSection(out, "ussd", "maplimits.ussd", true);
    startParams(out);

    java.util.List parseModes = new ArrayList();
    parseModes.add("onlyStar");
    parseModes.add("always");
    parseModes.add("never");

    param(out, "maplimits.ussd.no_sri_codes", "ussdNo_sri_codes", bean.getUssdNo_sri_codes());
    param(out, "maplimits.ussd.cond_sri_codes", "ussdCond_sri_codes", bean.getUssdCond_sri_codes());
    param(out, "maplimits.ussd.ati_codes", "ussdAti_codes", bean.getUssdAti_codes());
    param(out, "maplimits.ussd.openRespRealAddr", "ussdOpenRespRealAddr", bean.getUssdOpenRespRealAddr());
    param(out, "maplimits.ussd.parseAlways", "ussdParseAlways", bean.getUssdParseAlways());
    param(out, "maplimits.ussd.parseOnlyStar", "ussdParseOnlyStar", bean.getUssdParseOnlyStar());
    param(out, "maplimits.ussd.newerParse", "ussdParseNever", bean.getUssdParseNever());
    paramSelect(out, "maplimits.ussd.defaultParseMode", "ussdDefaultParsingMode", parseModes, bean.getUssdDefaultParsingMode());

    finishParams(out);
    finishSection(out);

    //################################## clevels #############################
    startSection(out, "clevels", "maplimits.clevels", true);
    MapLimits.Level[] levels = bean.getLevels();
    for( int ii = 1; ii <= 8; ii++ ) {
      MapLimits.Level l = levels[ii];
      startSection(out, "clevels.level"+ii, "maplimits.clevels.level"+ii, true);
      startParams(out);
      param(out, "maplimits.clevels.level.dialogsLimit", l.getDialogsLimitName(), Integer.toString(l.getDialogsLimit()));
      param(out, "maplimits.clevels.level.failUpperLimit", l.getFailUpperLimitName(), Integer.toString(l.getFailUpperLimit()));
      param(out, "maplimits.clevels.level.failLowerLimit", l.getFailLowerLimitName(), Integer.toString(l.getFailLowerLimit()));
      param(out, "maplimits.clevels.level.okToLower", l.getOkToLowerName(), Integer.toString(l.getOkToLower()));
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
