<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mcisme.beans.Profile,
		 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.mcisme.backend.Statistics,
                 ru.novosoft.smsc.mcisme.backend.DateCountersSet,
                 ru.novosoft.smsc.mcisme.backend.HourCountersSet"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Profile" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int rowN = 0;
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%
	String Address = bean.getAddress();
%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0 width="100%">
<col width="15%">
<col width="85%">
<tr class=row<%=rowN++&1%>>
  <th>Full Address</th>
  <td><input class=txt name=address value="<%=StringEncoderDecoder.encode(bean.getAddress())%>"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "mcisme.hint.query_profile");
page_menu_space(out);
page_menu_end(out);
        if(Address != null)
	{%>
<div class=content>
<table class=properties_list cellspacing=0 width="100%">
<col width="15%">
<col width="85%">
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Inform</th>
  <td><input class=check type=checkbox name=inform id=idInform value=true <%=bean.getInform() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Allow Notifications</th>
  <td><input class=check type=checkbox name=notify id=idNotify value=true <%=bean.getNotify() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Want notifications</th>
  <td><input class=check type=checkbox name=wantNotifyMe id=idWantNotifyMe value=true <%=bean.getWantNotifyMe() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Inform Template</th>
  <td>
  <select name=informTemplate id=idIT>
<%
	List informList = bean.getInformTemplates();
	for (Iterator i=informList.iterator(); i.hasNext();)
	{
		Profile.Identity id = (Profile.Identity)i.next();%>
		<option value="<%= id.id%>" <%= (id.id == bean.getSelectedInformId()) ? "selected":""%>>
		 <%=StringEncoderDecoder.encode(id.name)%></option><%
  	}
%>
  </select>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Notify Template</th>
  <td>
  <select name=notifyTemplate id=idIT>
<%
	List notifyList = bean.getNotifyTemplates();
	for (Iterator i=notifyList.iterator(); i.hasNext();)
	{
		Profile.Identity id = (Profile.Identity)i.next();%>
		<option value="<%= id.id%>" <%= (id.id == bean.getSelectedNotifyId()) ? "selected":""%>>
		 <%=StringEncoderDecoder.encode(id.name)%></option><%
  	}
%>
  </select>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Event Mask</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;&nbsp;Busy</th>
  <td><input class=check type=checkbox name=busy id=idBusy value=true <%=bean.getBusy() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;&nbsp;No Replay</th>
  <td><input class=check type=checkbox name=noReplay id=idNoReplay value=true <%=bean.getNoReplay() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;&nbsp;Absent</th>
  <td><input class=check type=checkbox name=absent id=idAbsent value=true <%=bean.getAbsent() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;&nbsp;Detach</th>
  <td><input class=check type=checkbox name=detach id=idDetach value=true <%=bean.getDetach() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>&nbsp;&nbsp;Unconditional</th>
  <td><input class=check type=checkbox name=uncond id=idUncond value=true <%=bean.getUncond() ? "checked" : ""%>></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbSet",  "common.buttons.set",  "mcisme.hint.set_profile");
page_menu_space(out);
page_menu_end(out);
	}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>