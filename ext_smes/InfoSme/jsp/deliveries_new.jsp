<%@ include file="/WEB-INF/inc/code_header.jsp"%>

<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.DeliveriesPage"%>

<jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.DeliveriesNew" />
<jsp:setProperty name="deliveries_bean" property="*"/>

<%
  int beanResult = deliveries_bean.process(request);
  switch(beanResult) {
    case InfoSmeBean.RESULT_STAT:
      response.sendRedirect("stat.jsp");
      return;
    case InfoSmeBean.RESULT_DLSTAT:
      response.sendRedirect("stat.jsp?csv=true");
      return;
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%
  int pageId = deliveries_bean.getActivePageId();

  if (pageId == DeliveriesPage.LOAD_FILE_PAGE) {
    FORM_URI = CPATH+"/upload";
    FORM_METHOD = "POST";
    FORM_ENCTYPE = "multipart/form-data";
  } else {
    FORM_URI = CPATH+"/esme_InfoSme/deliveries_new.jsp";
    FORM_ENCTYPE = "application/x-www-form-urlencoded";
  }

  if (request.getAttribute("multipart.request") != null)
    response.sendRedirect(request.getContextPath() + "/smsc/esme_InfoSme/deliveries_new.jsp");
%>

<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%if (request.isUserInRole(InfoSmeBean.INFOSME_ADMIN_ROLE)) {%>
<%@ include file="inc/header.jsp"%>
<%}%>

<%
  switch (pageId) {

    case DeliveriesPage.START_PAGE:
      %><%@ include file="deliveries_start.jsp"%><%
      break;
    case DeliveriesPage.PROCESS_FILE_PAGE:
      %><%@ include file="deliveries_process_file.jsp"%><%
      break;
    case DeliveriesPage.LOAD_FILE_PAGE:
      %>
      <input type="hidden" name="jsp" value="/esme_InfoSme/deliveries_new.jsp">
      <%@ include file="deliveries_file.jsp"%>
      <%
      break;
    case DeliveriesPage.EDIT_TASK_PAGE:
      %><%@ include file="deliveries_edit_task.jsp"%><%
      break;
    case DeliveriesPage.GEN_TASK_PAGE:
      %><%@ include file="deliveries_gen_task.jsp"%><%
      break;
    case DeliveriesPage.FINISH_TASK_PAGE:
      %><%@ include file="deliveries_finish_task.jsp"%><%
      break;
    default:
  }

%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>