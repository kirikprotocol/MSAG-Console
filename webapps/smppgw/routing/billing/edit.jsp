<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new billing rule</c:when>
      <c:otherwise>Edit billing rule "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>
  
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save rule"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel rule editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm-ep:properties title="Billing rule">
      <sm-ep:txt title="ID" name="id" readonly="${!bean.add}" validation="nonEmpty"/>
      <sm-ep:txtBox title="rule" name="rule"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>