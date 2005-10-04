<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.admin.provider.Provider,
                 ru.novosoft.smsc.admin.category.Category"%>
<%@ page import="ru.novosoft.smsc.admin.smsstat.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsstat.*"%>
<jsp:useBean id="routeDetailBean" scope="session" class="ru.novosoft.smsc.jsp.smsstat.RouteDetailBean" />
<%
	RouteDetailBean bean = routeDetailBean;
 //   bean.setFromDate(null);
 //   bean.setTillDate(null);
%>
<jsp:setProperty name="routeDetailBean" property="*"/>
<%
    TITLE = getLocString("stat.routTitle");
    MENU0_SELECTION = "MENU0_SMSSTAT";

    int beanResult = RouteDetailBean.RESULT_OK;
    switch(beanResult = bean.process(request))
    {
        case RouteDetailBean.RESULT_DONE:
             response.sendRedirect("routsdetail.jsp");
            return;
        case RouteDetailBean.RESULT_FILTER:
        case RouteDetailBean.RESULT_OK:
            break;
        case RouteDetailBean.RESULT_BACK:
             response.sendRedirect("index.jsp");
          break;

        case SmsStatFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%int rowN = 0;%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbBack",  "common.buttons.back",  "common.buttons.runBack");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>

<div class=page_subtitle><%=getLocString("stat.subTitle")%></div>
<table class=properties_list cellspacing=0>
<tr class=row0>
	<th><%=getLocString("common.util.FromDate")%>:</th>
	<td nowrap><%=bean.getDateFrom()%></td>
	<th><%=getLocString("common.util.TillDate")%>:</th>
	<td nowrap><%=bean.getDateTill()%></td>
</tr>
</table>
</div>
<table class=page_menu with="100%">
<%--<tr>
<td><input:select name="filterSelect" default="1"
    attributes="<%= as %>" options="<%= o %>"  /></td></tr> --%>

<tr><td align="left" with="10%"> &nbsp;&nbsp;&nbsp;<%=getLocString("common.util.FilterBy")%>: &nbsp;  </td><td align="left" with="80%">  &nbsp;  </td>

<%
page_small_menu_begin(out);
page_menu_button(session, out, "mbQuickFilter",  "common.buttons.apply",  "common.buttons.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter");
//page_menu_button(out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing", "clickCancel()");
//page_menu_space(out);
page_small_menu_end(out);
%>  <td align="left" with="1%">  &nbsp;  </td>
</tr>
<tr class=row<%=(rowN++)&1%>>
			<th>&nbsp;&nbsp;&nbsp;&nbsp;<%=getLocString("common.util.Provider")%>&nbsp;</th>
			<td><select name=providerId id=providerId >
				<option value="-1" ></option>
				<%for (Iterator iprov = bean.getProviders().iterator(); iprov.hasNext(); )
				{
					Provider provider = (Provider) iprov.next();
					String ProviderName=provider.getName();
          long ProviderId=provider.getId();
          String ProviderIdStr=String.valueOf(ProviderId);
          String encProviderName = StringEncoderDecoder.encode(ProviderName);
          if( bean.getProviderId() != null && ProviderId == bean.getProviderId().longValue() ) {
            %><option selected value="<%=ProviderId%>" ><%=encProviderName%></option><%
          } else {
					  %><option value="<%=ProviderId%>" ><%=encProviderName%></option><%
          }
				}
				%>
			</select></td>
		</tr>
    <tr class=row<%=(rowN++)&1%>>
			<th>&nbsp;&nbsp;&nbsp;&nbsp;<%=getLocString("common.util.Category")%>&nbsp;</th>
			<td><select name=categoryId id=categoryId >
				<option value="-1" ></option>
				<%for (Iterator jcat = bean.getCategories().iterator(); jcat.hasNext(); )
				{
				 Category category = (Category) jcat.next();
          long CategoryId=category.getId();
          String CategoryIdStr=String.valueOf(CategoryId);
					String CategoryName =category.getName();
          String encCategoryName = StringEncoderDecoder.encode(CategoryName);
          if( bean.getCategoryId() != null && CategoryId == bean.getCategoryId().longValue()) {
					  %><option selected value="<%=CategoryId%>" ><%=encCategoryName%></option><%
          } else {
					  %><option value="<%=CategoryId%>" ><%=encCategoryName%></option><%
          }
				}
				%>
			</select></td>
		</tr>
</table>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
//Statistics stat = bean.getStatistics();
int disNo = 1;
//if (stat != null) {
    //CountersSet total = stat.getTotal();
    //Collection dates = stat.getDateStat();
%>
<script language="JavaScript">
  function toggleVisible(p, c)
  {
    var o = p.className == "collapsing_list_opened";
    p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
    c.runtimeStyle.display = o ? "none" : "block";
  }
</script>
<table class=list cellspacing=0>
  <%
   // Collection routeids = stat.getRouteIdStat();
    ArrayList routeids = bean.getByRouteId();
   if (routeids!=null)
   {
    Iterator i = routeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=10> <div class=page_subtitle><%=getLocString("stat.routSubTitle")%></div></td>
</tr>
<tr class=row0>
    <th width="19%"><div align=right><%=getLocString("smsview.routeId")%></div></th>
    <th width="9%"><div align=right><%=getLocString("common.util.Provider")%></div></th>
    <th width="9%"><div align=right><%=getLocString("common.util.Category")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.accepted")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.rejected")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.delivered")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.failed")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.rescheduled")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.temporal")%></div></th>
    <th width="9%"><div align=right><%=getLocString("stat.peakInOut")%></div></th>
</tr>  <%
        while (i.hasNext()) {
            RouteIdCountersSet route = (RouteIdCountersSet)i.next();
            if (null==route) continue;
            String routeStr = StringEncoderDecoder.encode(route.routeid);
            Provider provider=route.getProvider();
            Category category=route.getCategory();
            String providerStr= provider.getName().equals("")  ? "&nbsp": StringEncoderDecoder.encode(provider.getName());
            String categoryStr= category.getName().equals("")  ? "&nbsp": StringEncoderDecoder.encode(category.getName());
            Collection errs = route.getErrors();
        %>
        <tr class=row0>
            <td width="19%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_closed><%= routeStr%><div></td>
            <td width="9%" align=right><%= providerStr %></td>
            <td width="9%" align=right><%= categoryStr %></td>
            <td width="9%" align=right><%= route.accepted%></td>
            <td width="9%" align=right><%= route.rejected%></td>
            <td width="9%" align=right><%= route.delivered%></td>
            <td width="9%" align=right><%= route.failed%></td>
            <td width="9%" align=right><%= route.rescheduled%></td>
            <td width="9%" align=right><%= route.temporal%></td>
            <td width="9%" align=right><%= route.peak_i%>&nbsp;/&nbsp;<%= route.peak_o%></td>
        </tr>
        <tr id="c<%=disNo++%>" style="display:none">
        <td colspan=8><table class=list cellspacing=0 cellpadding=0 border=0>
        <%
          Iterator ri = errs.iterator();
          while (ri.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)ri.next();
          %>
          <tr class=row1>
              <td width="23%" align=right nowrap>
          <%  String errMessage = getLocString("smsc.errcode."+errid.errcode);
              if (errMessage == null) errMessage = getLocString("smsc.errcode.unknown"); %>
          <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
          (<%=errid.errcode%>)
              </td>
              <td width="11%" align=right><%= errid.counter%></td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
          </tr><%
          }
        %>
        </table></td>
        </tr><%
        }
    } } %>
</table>
<%
//} // stat != null%>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

