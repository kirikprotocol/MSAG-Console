<%@ include file="/WEB-INF/inc/code_header.jsp"%>

<%@ page import="ru.novosoft.smsc.jsp.*"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizesList"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizesList" />
<jsp:setProperty name="bean" property="*"/>
<%

  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  bean.getTableHelper().processRequest(request);
	TITLE=getLocString("smsquiz.title");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
  switch (beanResult) {
      case PageBean.RESULT_OK :
      case PageBean.RESULT_ERROR :
      case PageBean.RESULT_DONE:
          break;
      case QuizesList.RESULT_ADD :
          response.sendRedirect("quizAdd.jsp");
          return;
      case QuizesList.RESULT_VIEW :
          response.sendRedirect("quizView.jsp?quiz=" + URLEncoder.encode(bean.getSelectedQuizId()));
          return;
      default :
          errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
  <%final PagedStaticTableHelper tableHelper = bean.getTableHelper();
    if(tableHelper.getSize()>0) {%>
      <%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
    <%}
    else {%>
      <div style="color:blue"><%=getLocString("smsquiz.message.no.quiz")%></div>
    <%}
    %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "smsquiz.quiz.add");
  if(tableHelper.getSize()>0) {
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "smsquiz.quiz.delete", getLocString("smsquiz.confirm.quiz.delete"));
  }
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>