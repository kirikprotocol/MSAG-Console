<jsp:useBean id="errorMessages" class="java.util.ArrayList"/><%@
page import="ru.novosoft.smsc.admin.Constants"
%><%
  request.setAttribute(Constants.SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME, errorMessages);
%>