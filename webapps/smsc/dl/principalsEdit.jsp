<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.route.Mask"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.dl.PrincipalsEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE = "Edit principal";
  MENU0_SELECTION = "MENU0_DL_PRINCIPALS";

  switch (bean.process(appContext, errorMessages, loginedUserPrincipal))
  {
    case dlEdit.RESULT_DONE:
      response.sendRedirect("principals.jsp?initialized=true");
      return;
    case dlEdit.RESULT_OK:
      STATUS.append("Ok");
      break;
    case dlEdit.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
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
	<th>address</th>
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
	<th>maximum lists</th>
	<td><input class=txt name=max_lst value="<%=bean.getMax_lst()%>" validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th>maximum elements</th>
	<td><input class=txt name=max_el value="<%=bean.getMax_el()%>" validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",  "Save",  "Save principal");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>