<%@ page import="ru.novosoft.smsc.util.SortedList,
                 java.util.Collection,
                 java.util.Comparator" %>
<%@ page import="java.util.Iterator"%>
<%= bean.isDefaultReshedule() ? getLocString("reschedule.defaultPolicy") : getLocString("reschedule.policy")%>: <input
        class=txt name=reshedule value="<%=bean.getReshedule()%>" validation="reschedule">
    <%
        if (!bean.isDefaultReshedule()) {
    %>
    <div class=page_subtitle><%=getLocString("common.titles.errors")%></div>
    <table class=properties_list>
        <col width=1>
        <col width=1 align=right>
        <%
            Collection allErrorCodes = new SortedList(bean.getAllErrCodes(), new Comparator() {
                public int compare(Object o1, Object o2) {
                    if (o1 instanceof String && o2 instanceof String) {
                        String s1 = (String) o1;
                        String s2 = (String) o2;
                        try {
                            return Long.decode(s1).compareTo(Long.decode(s2));
                        } catch (NumberFormatException e) {
                            return s1.compareTo(s2);
                        }
                    } else
                        return 0;
                }
            });
            int rowN = 0;
            for (Iterator i = allErrorCodes.iterator(); i.hasNext(); rowN++) {
                String errCode = (String) i.next();
                boolean isErrChecked = bean.isErrChecked(errCode);
        %><tr class=row<%=rowN & 1%>>
        <%
            if (isErrChecked || !bean.isErrorAssigned(errCode)) {%>
        <td><input id=checkedErrs_id_<%=i%> class=check type=checkbox name=checkedErrs
                   value=<%=errCode%> <%=isErrChecked ? "checked" : ""%>></td>
        <td><label for=checkedErrs_id_<%=i%>><%=errCode%></label></td>
        <td><label for=checkedErrs_id_<%=i%>><%=bean.getErrorString(errCode)%></label></td>
    </tr>
        <%} else {%>
        <td>&nbsp;</td>
        <td disabled><%=errCode%></td>
        <td disabled><%=bean.getErrorString(errCode)%></td>
    </tr>
    <%
            }
        }
    %>
</table>
<%}%>
