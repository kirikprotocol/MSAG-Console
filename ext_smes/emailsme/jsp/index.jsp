<%@ page import="ru.novosoft.smsc.emailsme.beans.Index,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.emailsme.backend.SmeContext"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = SmeContext.SME_ID;
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<div class=content>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApplyAll",  "Save",   "Save changes");
page_menu_button(out, "mbResetAll",  "Reset",  "Discard changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
