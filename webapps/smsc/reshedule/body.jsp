<%@ page import="ru.novosoft.smsc.jsp.smsc.reshedule.Body"%>
<%= bean.isDefaultReshedule() ? "Default reschedule policy" : "Reschedule policy"%>: <input class=txt name=reshedule value="<%=bean.getReshedule()%>">
<%
  if (!bean.isDefaultReshedule()) {
%>
<div class=page_subtitle>Errors</div>
<table class=properties_list>
<col width=1>
<col width=1 align=right>
<%
    for (int i = 0; i < bean.getAllErrCodes().length; i++) {
      String errCode = bean.getAllErrCodes()[i];
      boolean isErrChecked = bean.isErrChecked(errCode);
      %><tr class=row<%=i&1%>>
      <%
      if (isErrChecked || !bean.isErrorAssigned(errCode)) {%>
        <td><input id=checkedErrs_id_<%=i%> class=check type=checkbox name=checkedErrs value=<%=errCode%> <%=isErrChecked ? "checked" : ""%>></td>
        <td><label for=checkedErrs_id_<%=i%>><%=errCode%></label></td>
        <td><label for=checkedErrs_id_<%=i%>><%=bean.getErrorString(request.getLocale(), errCode)%></label></td>
        </tr>
      <%} else {%>
        <td>&nbsp;</td>
        <td disabled><%=errCode%></td>
        <td disabled><%=bean.getErrorString(request.getLocale(), errCode)%></td>
        </tr><%
      }
    }
%>
</table>
<%}%>
