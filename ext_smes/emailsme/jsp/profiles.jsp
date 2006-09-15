<%@ page import="ru.novosoft.smsc.emailsme.beans.Profiles"%>
<%@ page import="ru.novosoft.smsc.emailsme.backend.EmailSmeMessages"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Profiles" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);

  switch (beanResult)
  {
    case Profiles.RESULT_EDIT:
      if (bean.getFoundProfile() != null)
        response.sendRedirect("edit.jsp?addr=" + URLEncoder.encode(bean.getFoundProfile().getAddr(), "UTF-8") +
                                      "&userid=" + URLEncoder.encode(bean.getFoundProfile().getUserid(), "UTF-8") +
                                      "&dayLimit=" + bean.getFoundProfile().getDayLimit() +
                                      "&forward=" + URLEncoder.encode(bean.getFoundProfile().getForward(), "UTF-8")+
                                      "&realName=" + URLEncoder.encode(bean.getFoundProfile().getRealName(), "UTF-8"));
      return;
    case Profiles.RESULT_ADD:
      response.sendRedirect("edit.jsp?create=true");
      return;
    case Profiles.RESULT_DONE:
      response.sendRedirect("profiles.jsp");
      return;
  }
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
    <div class=content>
    <table>
      <tr>
        <td>
          <%=getLocString(EmailSmeMessages.messages.searchCriteria)%>
          <select name="searchField">
            <option value="<%=Profiles.SEARCH_BY_ADDR%>"/><%=getLocString(EmailSmeMessages.messages.address)%>
            <option value="<%=Profiles.SEARCH_BY_ID%>"/><%=getLocString(EmailSmeMessages.messages.userID)%>
          </select>
          <input type="text" name="searchString"/>
        </td>
      </tr>
    </table>
    </div>
    <%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSearch",    "Search",    "Search profile");
    page_menu_button(session, out, "mbAdd",    "Add",    "Add new profile");
    page_menu_space(out);
    page_menu_end(out);
    %>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
