<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.wsme.beans.WSmeVisitorsFormBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Welcome SME Visitors";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(errorMessages, loginedUserPrincipal))
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
    case WSmeFormBean.RESULT_FILTER:
      response.sendRedirect("visitors_filter.jsp");
      return;
    case WSmeFormBean.RESULT_VISITORS:
      response.sendRedirect("visitors.jsp?startPosition="+bean.getStartPosition()+
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
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/start_stop.jsp"%>
<br><%@ include file="inc/menu.jsp"%><br>
<div class=secView>Visitors</div>
<table class=secRep cellspacing=1 width="100%">
<thead>
<tr class=row0>
  <th><a href="#" <%=bean.getSort().endsWith("mask")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Mask" onclick='return setSort("mask")'>Mask</a></th>
</tr>
</thead>
<tbody>
<%
  int rowN=0;
  List visitors = bean.getVisitors();
  for (int i=0; i<visitors.size(); i++) {
    String mask = (String)visitors.get(i);
  %><tr class=row<%=rowN&1%>0>
      <td nowrap valign=top>
        <%if (bean.isWSmeStarted()) {%>
        <input class=check type=checkbox name=selectedRows value="<%= mask%>">&nbsp;
        <%}%><%= StringEncoderDecoder.encode(mask)%>
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
<div class=secQuestion>Manage visitors</div>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th class=label>Mask:</th>
	<td><input class=txtW type="text" name=newVisitor  value="<%= bean.getNewVisitor()%>" validation="mask" onkeyup="resetValidation(this)" size=25 maxlength=25></td>
</tr>
</table>

<div class=secButtons>
<input class=btn type=submit name=btnAdd value="Add new visitor" title="Add new visitor mask">
<input class=btn type=button name=btnDel value="Delete selected" title="Delete selected" onClick="noValidationSubmit(this)">
</div>
<%}%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
