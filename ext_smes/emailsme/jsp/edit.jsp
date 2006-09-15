<%@ page import="ru.novosoft.smsc.emailsme.beans.Edit,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ page import="ru.novosoft.smsc.emailsme.backend.EmailSmeMessages"%>

<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Edit" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
  int beanResult = bean.process(request);
  switch (beanResult) {
    case Edit.RESULT_DONE:
      response.sendRedirect("profiles.jsp");
      return;
    case Edit.RESULT_DELETE:
      response.sendRedirect("profiles.jsp");
      return;
  }
%>
<%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<table class=properties_list cellspacing=0>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString(EmailSmeMessages.messages.address)%></th>
  <td><input class=txt name=addr value="<%=StringEncoderDecoder.encode(bean.getAddr())%>" <%=bean.isCreate() ? "" : "readonly"%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString(EmailSmeMessages.messages.userID)%></th>
  <td><input class=txt name=userid value="<%=StringEncoderDecoder.encode(bean.getUserid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString(EmailSmeMessages.messages.dayLimit)%></th>
  <td><input class=txt name=dayLimit value="<%=StringEncoderDecoder.encode(bean.getDayLimit())%>" validation=positive onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString(EmailSmeMessages.messages.forwardAddress)%></th>
  <td><input class=txt name=forward value="<%=StringEncoderDecoder.encode(bean.getForward())%>"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "Save",   "Done editing");
page_menu_button(session, out, "mbDel",   "Delete",   "Delete");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
