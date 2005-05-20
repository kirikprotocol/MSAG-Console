<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.route.Mask"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.dl.PrincipalsEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE = getLocString("dl.editPrincipalTitle");
  MENU0_SELECTION = "MENU0_DL_PRINCIPALS";

  switch (bean.process(request))
  {
    case dlEdit.RESULT_DONE:
      response.sendRedirect("principals.jsp?initialized=true");
      return;
  }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<table class=properties_list cellspacing=0>
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th><%=getLocString("common.util.address")%></th>
	<td><%

    final String address = bean.getAddress();
    final String addressEnc = StringEncoderDecoder.encode(address);
    final String addressToShowEnc = address == null ? "" : StringEncoderDecoder.encode(new Mask(address).getMask());
    if (bean.isCreate())
    {
      %><input class=txt name=address value="<%=addressEnc%>" validation="address" onkeyup="resetValidation(this)"><%
    } else {
      %><input type=hidden name=address value="<%=addressEnc%>"><input class=txt name=addressToShow value="<%=addressToShowEnc%>" readonly><%
    }
	%></td>
</tr>
<tr class=row1>
	<th><%=getLocString("dl.maximumLists")%></th>
	<td><input class=txt name=max_lst value="<%=bean.getMax_lst()%>" validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th><%=getLocString("dl.maximumElements")%></th>
	<td><input class=txt name=max_el value="<%=bean.getMax_el()%>" validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",  "common.buttons.save",  "dl.savePrincipal");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>