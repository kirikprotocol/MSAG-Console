<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter"%>
<jsp:useBean id="wsme_lf_bean" scope="session" class="ru.novosoft.smsc.wsme.beans.WSmeLangsFilterFormBean" />
<%
  WSmeLangsFilterFormBean bean = wsme_lf_bean;
%>
<jsp:setProperty name="wsme_lf_bean" property="*"/>
<%
  TITLE="Welcome SME Langs Filter";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(errorMessages, loginedUserPrincipal))
  {
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("langs.jsp");
      return;
    case WSmeFormBean.RESULT_OK:
      STATUS.append("Ok");
      break;
    case WSmeFormBean.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<table class=properties_list cellspacing=0>
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 align=left><div class=page_subtitle>Masks</div></td></tr>
<%
  int row = 0;
  for (int i=0; i<bean.getMasks().length; i++)
  {
%>
<tr class=row<%=(row++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<th><input class=btn type=submit name=mbAddMask value="Add" title="Add new mask to filter"></th>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Langs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 align=left><div class=page_subtitle>Languages</div></td></tr>
<%
  row = 0;
  for (int i=0; i<bean.getLangs().length; i++)
  {
%>
<tr class=row<%=(row++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txt name=langs value="<%=bean.getLangs()[i]%>"></td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<th><input class=btn type=submit name=mbAddLang value="Add" title="Add new language to filter"></th>
	<td><input class=txt name=langs></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "return noValidationSubmit(this);");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "return noValidationSubmit(this);");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
