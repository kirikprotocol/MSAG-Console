<%@ include file="/WEB-INF/inc/code_header.jsp"%>

<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.DeliveriesPage"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.Deliveries" %>
<%@ page import="java.util.*" %>

<jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.deliveries.Deliveries" />
<jsp:setProperty name="deliveries_bean" property="*"/>

<%
  int beanResult = deliveries_bean.process(request);
  Deliveries bean = deliveries_bean;
  int pageId = deliveries_bean.getActivePageId();
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>

<%
  switch (pageId) {
    case DeliveriesPage.PROCESS_FILE_PAGE:%><%@ include file="deliveries_process_file.jsp"%><% break;
    case DeliveriesPage.LOAD_FILE_PAGE:%>   <%@ include file="deliveries_file.jsp"%><% break;
    case DeliveriesPage.EDIT_TASK_PAGE:%>   <%@ include file="deliveries_edit_task.jsp"%><% break;
    case DeliveriesPage.GEN_TASK_PAGE:%>    <%@ include file="deliveries_gen_task.jsp"%><% break;
    case DeliveriesPage.FINISH_TASK_PAGE:%> <%@ include file="deliveries_finish_task.jsp"%><% break;
    default:
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>