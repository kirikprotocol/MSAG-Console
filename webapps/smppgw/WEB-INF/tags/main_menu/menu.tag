<%@tag body-content="scriptless"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<table id=MAIN_MENU_TABLE cellspacing=0 cellpadding=0 class=main_menu>
  <tr>
    <td width=26></td>
    <jsp:doBody/>
    <td width="100%">&nbsp;</td>
    <%
      if (request != null && request.getUserPrincipal() != null) {
        %><td><a ID=MENU0_LOGOUT href="logout.jsp">Logout&nbsp;"<%=request.getUserPrincipal().getName()%>"</a></td><%
      }
    %>
    <td width=52></td>
  </tr>
</table>
