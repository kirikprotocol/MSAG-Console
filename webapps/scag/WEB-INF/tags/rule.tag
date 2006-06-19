<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@
 taglib prefix="sm-pm" tagdir="/WEB-INF/tags/page_menu"%>
<%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@tag body-content="empty"%>
<%@attribute name="transport" required="true"%>
<%@attribute name="exists" required="true"%>
     <script>
     function openJedit(id,transport,action)
     {
       window.open("<%=request.getContextPath()%>/rules/rules/jedit.jsp?id="+id+"&transport="+transport+"&action="+action,"","width=1,height=1,left="+screen.availWidth+",top="+screen.availHeight);
       return false;
     }
     </script>

  <sm-ep:property title = "${transport}">
     <c:choose>
        <c:when test="${exists}">
              <input type=button onClick="return openJedit('${bean.id}','<fmt:message>${transport}</fmt:message>','edit')" value="<fmt:message>service.edit.rule.edit</fmt:message>" > |
              <input type=submit name="deleteRule<fmt:message>${transport}</fmt:message>" value="<fmt:message>service.edit.rule.delete</fmt:message>">
        </c:when>
        <c:otherwise>
            <input type=button onClick="return openJedit('${bean.id}','<fmt:message>${transport}</fmt:message>','add')" value="<fmt:message>service.edit.rule.add</fmt:message>">
        </c:otherwise>
     </c:choose>
   </sm-ep:property>