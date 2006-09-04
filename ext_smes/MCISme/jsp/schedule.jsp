<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mcisme.beans.Schedule,
		 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.mcisme.backend.SchedItem,
                 java.util.*, java.text.SimpleDateFormat"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Schedule" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int rowN = 0;
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.query_abonents_record")%></div>
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
	if(Address != null)
	{
		if(bean.getSchedTime().length() == 0)
		{%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0 width="100%">
<col width="100%">
<tr class=row<%=rowN++&1%>>
  <th>Abonent <%=StringEncoderDecoder.encode(Address)%> is not in Delivery Queue now.</th>
</tr>
</table>
</div>
		<%}
		else
		{%>

<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0 width="100%">
<col width="25%">
<col width="25%">
<col width="25%">
<col width="25%">
<tr class=row<%=rowN++&1%>>
  <th>Schedule Time</th>
  <td>Address</td>
  <td>Events Count</td>
  <td>Last Error</td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=StringEncoderDecoder.encode(bean.getSchedTime())%></th>
  <td><%=StringEncoderDecoder.encode(bean.getAddress())%></td>
  <td><%=StringEncoderDecoder.encode(bean.getEventsCount())%></td>
  <td><%=StringEncoderDecoder.encode(bean.getLastError())%></td>
</tr>
</table>
</div>
		<%}
	}
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "mcisme.hint.query_profile");
page_menu_space(out);
page_menu_end(out);
%>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.first_50_records_in_delivery_queue")%></div>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0 width="100%">
<col width="25%">
<col width="25%">
<col width="25%">
<col width="25%">
<tr class=row<%=rowN++&1%>>
  <th>Schedule Time</th>
  <td>Abonents Count</td>
</tr>
<%
	List abonentsList = bean.getAbonentsList();
	for (Iterator i=abonentsList.iterator(); i.hasNext();)
	{
		SchedItem item = (SchedItem)i.next();
%>
<tr class=row<%=rowN++&1%>>
  <th><%=StringEncoderDecoder.encode(item.schedTime)%></th>
  <td><%=StringEncoderDecoder.encode(item.abonentsCount)%></td>
</tr>
<%
  	}
%>
</table>
</div>
<%
//page_menu_begin(out);
//page_menu_button(session, out, "mbRefresh",  "common.buttons.refresh",  "mcisme.hint.refresh_records");
//page_menu_space(out);
//page_menu_end(out);
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>