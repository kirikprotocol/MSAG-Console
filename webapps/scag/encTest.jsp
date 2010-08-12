<%@include file="/WEB-INF/inc/header.jspf"%>
<c:if test="${!empty param.locale}">
  <fmt:setLocale value="${param.locale}" scope="session"/>
</c:if>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
   <head>
     <title>Character encoding test page</title>
   </head>
   <body>
     <p>Data posted to this form was:
     <%
       request.setCharacterEncoding("UTF-8");
       out.print(request.getParameter("mydata"));
     %>

     </p>
     <form method="POST" action="encTest.jsp">
       <input type="text" name="mydata">
       <input type="submit" value="Submit" />
       <input type="reset" value="Reset" />
     </form>  
     <fmt:requestEncoding value="utf-8" />   
   <h1><fmt:message>common.title</fmt:message></h1>
   </body>
</html>