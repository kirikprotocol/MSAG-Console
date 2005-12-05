<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Offset,
				 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
			     java.util.Set, java.util.Iterator, ru.novosoft.smsc.util.StringEncoderDecoder,
				 java.util.Collection, ru.novosoft.smsc.mcisme.beans.MCISmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Offset" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("mcisme.title");
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
  String offsetPageSubtitle = getLocString("mcisme.subtitle.tz_offset_" + ((bean.isCreateOffset() ? "c":"m")));
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
  <th><%= getLocString("mcisme.label.tzo_name")%></th>
  <td><input class=txt name=offsetName value="<%=StringEncoderDecoder.encode(bean.getOffsetName())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th><%= getLocString("mcisme.label.tzo_value")%></th>
  <td><input class=txt name=offset value="<%=StringEncoderDecoder.encode(bean.getOffset())%>" maxlength="10" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th valign=top>mcisme.label.tzo_re</th>
  <td valign=top><textarea name=regexp><%=StringEncoderDecoder.encode(bean.getRegexp())%></textarea></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "mcisme.hint.done_edit");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "mcisme.hint.cancel_changes");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>