<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Routes Tracing">
  <jsp:attribute name="menu">
  </jsp:attribute>

  <jsp:body>
   <%-- <sm:table columns="checkbox,name,notes" names="c,name,notes" widths="1,49,50" edit="name"/>
    <%-- <c:set var="smes" value="${bean.smeIds}"/>  --%>

  <c:choose>
    <c:when test="${bean.appContext.statuses.routesLoaded}">Create new subject

   <sm-ep:properties title="Routes tracing">
      <sm-ep:txt title="Source Address" name="srcAddress"   validation="nonEmpty"/>
      <sm-ep:txt title="Destination Address" name="dstAddress"  />
      <sm-ep:txt title="Source System ID" name="srcSysId"  />
    </sm-ep:properties>

    <sm:break>
    <sm-pm:menu>
      <sm-pm:item name="mbCheck" value="Loadup & Check" title="Load & Check routing configuration"/>
      <sm-pm:item name="mbTrace" value="Trace route" title="Trace route" />
      <sm-pm:space/>
    </sm-pm:menu>
    </sm:break>

   </c:when>
   <c:otherwise>Trace Rout


    <div class=content>
    <table class=properties_list cell>
      <tr class=row0><td>
       <span class="C800"><b>Warning !</b></span><br><br>
       <span class="C000">Active routing configuration is not loaded or has been changed.</span><br>
       <span class="C000">Please loadup new routing information to route tracer.</span><br>&nbsp;
      </td></tr>
    </table>
    </div>

    <sm:break>
     <sm-pm:menu>
      <sm-pm:item name="mbCheck" value="Loadup & Check" title="Load & Check routing configuration"/>
      <sm-pm:space/>
     </sm-pm:menu>
    </sm:break>

  </c:otherwise>
 </c:choose>

</jsp:body>
</sm:page>


