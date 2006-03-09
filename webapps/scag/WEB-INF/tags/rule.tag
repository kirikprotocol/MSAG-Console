<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@
 taglib prefix="sm-pm" tagdir="/WEB-INF/tags/page_menu"%>
<%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@tag body-content="empty"%>
<%@attribute name="transport" required="true"%>
<%@attribute name="exists" required="true"%>
     <script>
        function newRule(id,transport)
        {
            document.jedit.newRule(id,transport);
            return false;
        }
        function editRule(id,transport)
        {
            document.jedit.openRule(id,transport);
            return false;
        }
     </script>

  <sm-ep:property title = "${transport}">
     <c:choose>
        <c:when test="${exists}">
             <a href="#" onClick="return editRule('${bean.id}','${transport}');"> Edit </a> |
             <%--sm-pm:item name="editRule" value="Edit" title="Edit Rule" onclick="editRule('${bean.id}','${transport}')"/--%>
              <input type=submit id="deleteRule${transport}" name="deleteRule${transport}" value="Delete">
              </input>
        </c:when>
        <c:otherwise>
              <%--input type=submit id="addRule${transport}" name="addRule${transport}" value="Add">
              </input--%>
             <a href="#" onClick="return newRule('${bean.id}','${transport}');"> Add </a>

        </c:otherwise>
     </c:choose>
   </sm-ep:property>
