<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.dl.dlFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("dl.filterTitle");
  MENU0_SELECTION = "MENU0_DL";

  int beanResult = PageBean.RESULT_OK;
  switch(beanResult = bean.process(request)) {
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
    case dlFilter.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Names ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("common.titles.names")%></div>
<table class=list cellspacing=0>
<col width="15%">
<col width="85%">
<%
for (int i=0; i<bean.getNames().length; i++)
{
%>
<tr>
	<td colspan=2><input class=txt name=names value="<%=bean.getNames()[i]%>"></td>
</tr>
<%}%>
<tr>
	<td><input class=txt name=names></td>
	<td><%addButton(out, "mbAdd", "Add", "dl.addNameHint");%></td>
</tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Owners ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("common.titles.owners")%></div>
<table class=list cellspacing=0>
<col width="15%">
<col width="85%">
<%
  for (int i=0; i<bean.getOwners().length; i++)
  {
%>
<tr>
	<td colspan=2><input class=txt name=owners value="<%=bean.getOwners()[i]%>"></td>
</tr>
<%}%>
<tr>
	<td><input class=txt name=owners></td>
	<td><%addButton(out, "mbAdd", "Add", "dl.addOwnerHint");%></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "dl.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>