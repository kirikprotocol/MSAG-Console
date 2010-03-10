<%@ page import="ru.novosoft.smsc.infosme.beans.Messages"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="infoSmeMessagesBean" scope="session" class="ru.novosoft.smsc.infosme.beans.Messages" />
<%--<% if (!bean.isProcessed()) {%>--%>
  <jsp:setProperty name="infoSmeMessagesBean" property="*"/>
<%--<% } %>--%>
<%
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  Messages bean = infoSmeMessagesBean;

  int beanResult = bean.process(request);

  if (beanResult == Messages.RESULT_UPDATE || beanResult == Messages.RESULT_CANCEL_UPDATE) {
    beanResult = PageBean.RESULT_OK;
    request.getRequestDispatcher("messages.jsp").forward(request, response);
    return;
  } else {
    beanResult = InfoSmeBean.RESULT_OK;
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="inc/header.jsp"%>

<input type=hidden name=initialized value=true>
<input type="hidden" name="taskId" id="taskId" value="<%=bean.getTaskId()%>"/>
<input type="hidden" name="status" id="status" value="<%=bean.getStatus()%>"/>
<input type="hidden" name="fromDate" id="fromDate" value="<%=bean.getFromDate()%>"/>
<input type="hidden" name="tillDate" id="tillDate" value="<%=bean.getTillDate()%>"/>
<input type="hidden" name="address" id="address" value="<%=bean.getAddress()%>"/>

<table class="list">
  <tr class="row0">
    <td align=left nowrap><%=getLocString("infosme.messages.update.message")%></td>
    <td align=left width="100%"><input class="txtW" type="text" name="message2update" id="message2update"/></td>
  </tr>
</table>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbUpdate",    "infosme.button.update", "");
  page_menu_button(session, out, "mbCancelUpdate",    "common.buttons.cancel", "");
  page_menu_space(out);
  page_menu_end(out);
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>