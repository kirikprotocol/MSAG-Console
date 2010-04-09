<%@ include file="/WEB-INF/inc/code_header.jsp"%>

<%@ page import="ru.novosoft.smsc.jsp.*"%>
<%@ page import="mobi.eyeline.welcomesms.beans.Networks"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.novosoft.smsc.util.Functions"%>
<%@ page import="java.util.Iterator"%>
<%@ page import="java.util.Map"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.welcomesms.beans.Networks" />
<jsp:setProperty name="bean" property="*"/>
<%

  bean.getTableHelper().processRequest(request);
	TITLE=getLocString("welcomesms.title")+": "+getLocString("welcomesms.label.networks");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
  switch (beanResult) {
      case PageBean.RESULT_OK :
      case PageBean.RESULT_ERROR :
      case PageBean.RESULT_DONE:
          break;
      case Networks.RESULT_ADD :
          response.sendRedirect("networkEdit.jsp");
          return;
      case Networks.RESULT_EDIT :
          response.sendRedirect("networkEdit.jsp?id=" + URLEncoder.encode(bean.getSelectedId()));
          return;
      default :
          errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
  <table>
 <% int rowN = 0;   %>
    <tr>
      <th style="text-align:left"><%= getLocString("welcomesms.network.name")%></th>
      <td><input class=txt id="name" name="name"  value="<%=bean.getName()%>" size=25 maxlength=25></td>
      <th style="text-align:left"><%= getLocString("welcomesms.network.mcc")%></th>
      <td><input class=txt id=mcc name=mcc value="<%=StringEncoderDecoder.encode(bean.getMcc())%>" maxlength=3></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("welcomesms.network.country")%></th>
    <td><select name=countryId>
      <option value="" <%= (bean.getCountryId() == null || bean.getCountryId().equals("")) ? "selected":""%>>All</option>
      <%
        for (Iterator i = bean.getCountries().entrySet().iterator(); i.hasNext();) {
          Map.Entry e = (Map.Entry)i.next();
          String countryId = (String)e.getKey();
          String country = (String)e.getValue();
          String countryEnc = StringEncoderDecoder.encode(country);
      %><option value="<%=countryId%>" <%= countryId.equals(bean.getCountryId()) ? "selected":""%>><%=countryEnc %></option>
      <%}%> </select></td>
      <th style="text-align:left"><%= getLocString("welcomesms.network.mnc")%></th>
      <td><input class=txt id=mnc name=mnc value="<%=StringEncoderDecoder.encode(bean.getMnc())%>" maxlength=2></td>
    </tr>
  </table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "common.buttons.query");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>

  <%final PagedStaticTableHelper tableHelper = bean.getTableHelper();
    if(tableHelper.getSize()>0) {%>
  <%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
    <%}
    else {%>
  <div style="color:blue"><%=getLocString("welocmesms.message.no.network")%></div>
    <%}
    %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "common.buttons.add");
  if(tableHelper.getSize()>0) {
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "common.buttons.delete", getLocString("welocmesms.confirm.network.delete"));
  }
  page_menu_space(out);

  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>