<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="a_bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeAdsFormBean" />
<jsp:setProperty name="a_bean" property="*"/>
<%
  TITLE="Welcome SME Messages";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = a_bean.RESULT_OK;
  switch(beanResult = a_bean.process(errorMessages))
  {
    case WSmeFormBean.RESULT_VISITORS:
      response.sendRedirect("visitors.jsp");
      return;
    case WSmeFormBean.RESULT_LANGS:
      response.sendRedirect("langs.jsp");
      return;
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
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

<%@ include file="inc/menu.jsp"%>

<div class=secView>Messages</div>
<table class=secRep cellspacing=1 width="100%">
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th>Id</th>
  <th>Language</th>
  <th>Message</th>
</tr>
</thead>
<tbody>
<%
  int rowN=0;
  List ads = a_bean.getAds();
  for (int i=0; i<ads.size(); i++) {
    AdRow ad = (AdRow)ads.get(i);
  %><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <input class=check type=checkbox name=selectedRows value="<%= ""+ad.id+"-"+ad.lang%>">
      </td>
      <td nowrap valign=top>
        <%= ad.id%>
      </td>
      <td nowrap valign=top>
        <%= StringEncoderDecoder.encode(ad.lang)%>
      </td>
      <td nowrap valign=top>
        <%= StringEncoderDecoder.encode(ad.ad)%>
      </td>
    </tr>
  <%
  }
%>
</tbody>
</table>

<div class=secQuestion>Manage messages</div>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th class=label>Id:</th>
	<td><input class=txtW type="text" name=newId  value="<%= a_bean.getNewId()%>" size=25 maxlength=25></td>
</tr>
<tr class=row1>
	<th class=label>Language:</th>
	<td><input class=txtW type="text" name=newLang  value="<%= a_bean.getNewLang()%>" size=25 maxlength=25></td>
</tr>
<tr class=row0>
	<th class=label>Message:</th>
	<td><input class=txtW type="text" name=newAd  value="<%= a_bean.getNewAd()%>" size=25 maxlength=25></td>
</tr>
</table>

<div class=secButtons>
<input class=btn type=submit name=btnAdd value="Add new message" title="Add new message">
<input class=btn type=submit name=btnDel value="Delete selected" title="Delete selected">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
