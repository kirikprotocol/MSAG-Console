<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="v_bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeVisitorsFormBean" />
<jsp:setProperty name="v_bean" property="*"/>
<%
  TITLE="Welcome SME Visitors";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = v_bean.RESULT_OK;
  switch(beanResult = v_bean.process(errorMessages))
  {
    case WSmeFormBean.RESULT_LANGS:
      response.sendRedirect("langs.jsp");
      return;
    case WSmeFormBean.RESULT_ADS:
      response.sendRedirect("ads.jsp");
      return;
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case WSmeFormBean.RESULT_VISITORS:
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

<div class=secView>Visitors</div>
<table class=secRep cellspacing=1 width="100%">
<thead>
<tr class=row0>
  <th>&nbsp;</th>
  <th>Mask</th>
</tr>
</thead>
<tbody>
<%
  int rowN=0;
  List visitors = v_bean.getVisitors();
  for (int i=0; i<visitors.size(); i++) {
    String mask = (String)visitors.get(i);
  %><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <input class=check type=checkbox name=selectedRows value="<%= mask%>">
      </td>
      <td nowrap valign=top>
        <%= StringEncoderDecoder.encode(mask)%>
      </td>
    </tr>
  <%
  }
%>
</tbody>
</table>

<div class=secQuestion>Manage visitors</div>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th class=label>Mask:</th>
	<td><input class=txtW type="text" name=newVisitor  value="<%= v_bean.getNewVisitor()%>" size=25 maxlength=25></td>
</tr>
</table>

<div class=secButtons>
<input class=btn type=submit name=btnAdd value="Add new visitor" title="Add new visitor mask">
<input class=btn type=submit name=btnDel value="Delete selected" title="Delete selected">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
