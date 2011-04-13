<%@ page import="ru.sibinco.sponsored.stats.beans.CreateSponsorRequestBean" %>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.sponsored.stats.beans.CreateSponsorRequestBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("sponsor.stats.request.create.title");
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
	switch(beanResult)
	{
		case CreateSponsorRequestBean.RESULT_SAVE:
		case CreateSponsorRequestBean.RESULT_CANCEL:
			response.sendRedirect("index.jsp");
			return;
		default:
	}
  int rowN = 0;
%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>

<div class=content>

  <div class=page_subtitle><%=getLocString("sponsor.request.new")%></div>
  <table style="padding-top:5px" class=properties_list>
    <col width="1%">
    <col width="99%">
    <tr>
      <th><%=getLocString("smsx.stat.from")%></th>
      <td><input type=text id="from" name="from" class=calendarField
                 value="<%=bean.getFrom()%>" maxlength=10 style="z-index:22;"><button
          class=calendarButton type=button onclick="return showCalendar(from, false, false);">...</button></td>
      <td>&nbsp;</td>
    </tr>
    <tr>
      <th><%=getLocString("smsx.stat.till")%></th>
      <td><input type=text id="till" name="till" class=calendarField
                 value="<%=bean.getTill()%>" maxlength=10 style="z-index:22;"><button
          class=calendarButton type=button onclick="return showCalendar(till, false, false);">...</button></td>
      <td>&nbsp;</td>
    </tr>
    <tr>
      <th><%=getLocString("smsx.stat.bonus")%></th>
      <td><input validation="nonEmpty" maxlength="10" size="10" type=text id="bonus" name="bonus" value="<%=bean.getBonus()%>"/></td>
      <td>&nbsp;</td>
    </tr>
  </table>
</div>


<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "smsx.stat.request.create", "smsx.stat.request.create");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>

<script type="text/javascript">
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>