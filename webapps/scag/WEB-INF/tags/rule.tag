<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@taglib uri="/scag/func" prefix="smf"%>
<%@
 taglib prefix="sm-pm" tagdir="/WEB-INF/tags/page_menu"%>
<%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@tag body-content="empty"%>
<%@attribute name="tname" required="true"%>
<%@attribute name="ttitle" required="true"%>
<%@attribute name="exists" required="true"%>
<%@attribute name="locked" required="true"%>

  <sm-ep:property title = "${ttitle}">
     <c:choose>
        <c:when test="${exists}">
             <input type=submit <c:if test="${locked}">disabled</c:if> onClick="return openJedit('${smf:getComplexRuleId(bean.id,tname)}','edit')" value="<fmt:message>service.edit.rule.edit</fmt:message>"> |
             <input type=submit <c:if test="${locked}">disabled</c:if> name="deleteRule${tname}" value="<fmt:message>service.edit.rule.delete</fmt:message>">
        </c:when>
        <c:otherwise>
             <input type=submit <c:if test="${locked}">disabled</c:if> onClick="return openJedit('${smf:getComplexRuleId(bean.id,tname)}','add')" value="<fmt:message>service.edit.rule.add</fmt:message>">
        </c:otherwise>
     </c:choose>
  </sm-ep:property>