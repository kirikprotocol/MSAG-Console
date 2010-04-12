<%@ page import="java.util.Iterator"%>
<%@ page import="java.util.Map"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.welcomesms.beans.NetworkEditBean" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE = getLocString("welcomesms.title")+": "+(bean.getId() == null ? getLocString("welcomesms.network.add")
  :getLocString("welcomesms.network.edit"));
  int beanResult = bean.process(request);
  if(beanResult == WelcomeSMSBean.RESULT_DONE) {
    response.sendRedirect("networks.jsp");
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>


<input type=hidden name=initialized value=true>
<input type=hidden name=id value="<%=bean.getId() == null ? "" : bean.getId()%>">

<div class=content>
<% int rowN = 0;%>
  <table class=properties_list cellspacing=0>
    <col width="1%">
    <col width="99%">
    <tr class=row<%=rowN++&1%>>
      <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("welcomesms.label.network")%></div></th>
    </tr>

    <tr class=row<%=rowN++&1%>>
      <th>
      <%=getLocString("welcomesms.network.name")%>
      </th>
      <td>
      <input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>">
      </td>
    </tr>

    <tr class=row<%=rowN++&1%>>
      <th><%=getLocString("welcomesms.network.country")%></th>
      <td><select name=countryId>
      <%
        for (Iterator i = bean.getCountries().entrySet().iterator(); i.hasNext();) {
          Map.Entry e = (Map.Entry)i.next();
          String countryId = (String)e.getKey();
          String country = (String)e.getValue();
          String countryEnc = StringEncoderDecoder.encode(country);
      %><option value="<%=countryId%>" <%= countryId.equals(bean.getCountryId()) ? "selected":""%>><%=countryEnc %></option>
      <%}%> </select></td>
    </tr>


    <tr class=row<%=rowN++&1%>>
      <th><%=getLocString("welcomesms.network.mcc")%></th>
      <td>
      <input class=txt name=mcc value="<%=StringEncoderDecoder.encode(bean.getMcc())%>"></td>
    </tr>

    <tr class=row<%=rowN++&1%>>
      <th><%=getLocString("welcomesms.network.mnc")%></th>
      <td>
      <input class=txt name=mnc value="<%=StringEncoderDecoder.encode(bean.getMnc())%>"></td>
    </tr>


</table>

</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbDone",   "common.buttons.done",  "common.buttons.done");
  page_menu_button(session, out, "mbCancel",   "common.buttons.cancel",  "common.buttons.cancel");
  page_menu_space(out);
  page_menu_end(out);

%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>