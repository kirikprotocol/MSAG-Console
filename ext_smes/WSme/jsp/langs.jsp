<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeLangsFormBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Constants.WSME_SME_ID;
  TITLE="Welcome SME Languages";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(errorMessages, loginedUserPrincipal))
  {
    case WSmeFormBean.RESULT_VISITORS:
      response.sendRedirect("visitors.jsp");
      return;
    case WSmeFormBean.RESULT_ADS:
      response.sendRedirect("ads.jsp");
      return;
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case WSmeFormBean.RESULT_FILTER:
      response.sendRedirect("langs_filter.jsp");
      return;
    case WSmeFormBean.RESULT_LANGS:
      response.sendRedirect("langs.jsp?startPosition="+bean.getStartPosition()+
          "&pageSize="+bean.getPageSize());
      return;
    case WSmeFormBean.RESULT_OK:
      STATUS.append("Ok");
      break;
    case WSmeFormBean.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/start_stop.jsp"%>
<%@ include file="inc/menu.jsp"%>
<div class=content>
<div class=page_subtitle>Languages</div>
<table class=list cellspacing=0>
<thead>
<tr class=row0>
  <th><a href="#" <%=bean.getSort().endsWith("mask")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Mask" onclick='return setSort("mask")'>Mask</a></th>
  <th><a href="#" <%=bean.getSort().endsWith("lang")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Language" onclick='return setSort("lang")'>Language</a></th>
</tr>
</thead>
<tbody>
<%
  int rowN=0;
  List langs = bean.getLangs();
  for (int i=0; i<langs.size(); i++) {
    LangRow lang = (LangRow)langs.get(i);
  %><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <%if (bean.isWSmeStarted()) {%>
        <input class=check type=checkbox name=selectedRows value="<%= lang.mask%>">&nbsp;
        <%}%><%= StringEncoderDecoder.encode(lang.mask)%>
      </td>
      <td nowrap valign=top>
        <%= StringEncoderDecoder.encode(lang.lang)%>
      </td>
    </tr>
  <%
  }
%>
</tbody>
</table>

<input type=hidden name=sort value="<%=bean.getSort()%>">
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=pageSize value=<%=bean.getPageSize()%>>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>

<%if (bean.isWSmeStarted()) {%>
<br>
<div class=page_subtitle>Manage languages</div>
<table class=properties_list cellspacing=0>
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th>Mask:</th>
	<td><input class=txt type="text" name=newMask  value="<%= StringEncoderDecoder.encode(bean.getNewMask())%>" validation="mask" onkeyup="resetValidation(this)" size=25 maxlength=25></td>
</tr>
<tr class=row1>
	<th>Language:</th>
	<td><input class=txt type="text" name=newLang  value="<%= StringEncoderDecoder.encode(bean.getNewLang())%>" size=25 maxlength=25></td>
</tr>
</table>
<%}%>
</div>
<%
page_menu_begin(out);
if (bean.isWSmeStarted()) {
	page_menu_button(out, "btnAdd",  "Add new language",  "Add new language");
	page_menu_button(out, "btnDel",  "Delete selected",  "Delete selected languages",  "return noValidationSubmit(this);");
}
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
