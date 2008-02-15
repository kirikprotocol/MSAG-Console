<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@ tag body-content="scriptless"%><%@attribute name="menu" required="false"%><%@attribute name="menu2" required="false"%>
${menu}

<%!
 boolean isNotNull = false;
%>

<%
    if(session.getAttribute("BROWSER_TYPE") != null){
        isNotNull = true;
    }
%>
<%  if( isNotNull && session.getAttribute("BROWSER_TYPE").equals("IE") ){
//        if(session.getAttribute("BROWSER_TYPE").equals("IE")){
%>
            <div class=content>
                <jsp:doBody/>
            </div>
<%
//        }else{
%>
<%--            <div class=contentFF>--%>
<%--                <jsp:doBody/>--%>
<%--            </div>--%>
<%
//        }
    } else {
%>
            <div class=contentFF>
                <jsp:doBody/>
            </div>
<%
    }
%>

<c:choose>
  <c:when test="${!empty menu2}">${menu2}</c:when>
  <c:otherwise>${menu}</c:otherwise>
</c:choose>
