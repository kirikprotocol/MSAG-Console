<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="l_bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeLangsFormBean" />
<jsp:setProperty name="l_bean" property="*"/>
<%
  TITLE="Welcome SME Languages";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = l_bean.RESULT_OK;
  switch(beanResult = l_bean.process(errorMessages))
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
    case WSmeFormBean.RESULT_LANGS:
      STATUS.append("Ok");
      break;
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

<%@ include file="inc/menu.jsp"%>

<div class=secView>Languages</div>
<table class=secRep cellspacing=1 width="100%">
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th>Mask</th>
  <th>Language</th>
</tr>
</thead>
<tbody>
<%
  int rowN=0;
  List langs = l_bean.getLangs();
  for (int i=0; i<langs.size(); i++) {
    LangRow lang = (LangRow)langs.get(i);
  %><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <input class=check type=checkbox name=selectedRows value="<%= lang.mask%>">
      </td>
      <td nowrap valign=top>
        <%= StringEncoderDecoder.encode(lang.mask)%>
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

<div class=secQuestion>Manage languages</div>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th class=label>Mask:</th>
	<td><input class=txtW type="text" name=newMask  value="<%= l_bean.getNewMask()%>" size=25 maxlength=25></td>
</tr>
<tr class=row1>
	<th class=label>Language:</th>
	<td><input class=txtW type="text" name=newLang  value="<%= l_bean.getNewLang()%>" size=25 maxlength=25></td>
</tr>
</table>

<div class=secButtons>
<input class=btn type=submit name=btnAdd value="Add new language" title="Add new language">
<input class=btn type=submit name=btnDel value="Delete selected" title="Delete selected">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
