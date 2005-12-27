<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.reshedule.Add" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.reshedule.Add" />
<jsp:setProperty name="bean" property="*" />
<%
  FORM_METHOD = "POST";
  TITLE = getLocString("reschedule.title");
  MENU0_SELECTION = "MENU0_SMSC_Reshedule";

  switch (bean.process(request)) {
    case Add.RESULT_DONE :
      response.sendRedirect("index.jsp");
      return;
    case Add.RESULT_OK :
      break;
    case Add.RESULT_ERROR :
      break;
    default :
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
  <%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
  <%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave", "common.buttons.done", "common.buttons.saveConfig");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
  <%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
  <div class=content>
    <%@ include file="body.jsp" %>
  </div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave", "common.buttons.done", "common.buttons.saveConfig");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
  <%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
  <%@ include file="/WEB-INF/inc/code_footer.jsp" %>
