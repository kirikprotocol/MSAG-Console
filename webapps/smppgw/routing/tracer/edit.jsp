<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new subject</c:when>
      <c:otherwise>Edit subject "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>
  
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save subject"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel subject editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <c:set var="smes" value="${bean.smeIds}"/>
    <sm-ep:properties title="Routes tracing">
      <sm-ep:txt title="name" name="name" readonly="${!bean.add}" validation="nonEmpty"/>
      <sm-ep:list title="default SME" name="defaultSme" values="${smes}" valueTitles="${smes}"/>
      <sm-ep:elist title="masks" name="masks" validation="mask"/>
      <sm-ep:txt title="notes" name="notes"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>