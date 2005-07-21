<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Offset,
				 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
			     java.util.Set, java.util.Iterator, ru.novosoft.smsc.util.StringEncoderDecoder,
				 java.util.Collection, ru.novosoft.smsc.mcisme.beans.MCISmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Offset" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Missed Calls Info SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch(beanResult)
  {
	case MCISmeBean.RESULT_DONE:
		response.sendRedirect("offsets.jsp");
		return;
	default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
  int rowN=1;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%><%
  String offsetPageSubtitle = "TimeZone offset "+(bean.isCreateOffset() ? "create":"modify");
%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=createOffset  value=<%=bean.isCreateOffset()%>>
<input type=hidden name=oldOffsetName value="<%=bean.getOldOffsetName()%>">
<div class=page_subtitle><%= StringEncoderDecoder.encode(offsetPageSubtitle)%></div>
<table class=properties_list cellspacing=0 cellpadding=4 width="100%" id="offsetTable">
<col width="1%" align=left>
<col width="99%" align=left>
<tr class=row<%=(rowN++)&1%>>
  <th>Time offset name</th>
  <td><input class=txt name=offsetName value="<%=StringEncoderDecoder.encode(bean.getOffsetName())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th>Offset (in hours)</th>
  <td><input class=txt name=offset value="<%=StringEncoderDecoder.encode(bean.getOffset())%>" maxlength="10" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th valign=top>Reg-Exp</th>
  <td valign=top><textarea name=regexp><%=StringEncoderDecoder.encode(bean.getRegexp())%></textarea></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",   "Done",   "Submit changes");
page_menu_button(out, "mbCancel", "Cancel", "Discard changes");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>