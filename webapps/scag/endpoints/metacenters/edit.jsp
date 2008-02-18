<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">metacenter.add.title</c:when>
            <c:otherwise>metacenter.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="metaeps.edit.item.mbsave.value" title="metacenter.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="metaeps.edit.item.mbcancel.value" title="metacenter.edit.item.mbcancel.title"
                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <script>
    var global_counter = 0;
  function removeSmeRow( sme, rowId ){
      tbl = getElementByIdUni("smeTable");
      var rowElem = tbl.rows[rowId];
      var selectElem = getElementByIdUni('smeSelect');
      var oOption = document.createElement("OPTION");
      selectElem.options.add(oOption);
      var elem = getElementByIdUni('selected_sme_'+sme);
      var smeValue = elem.value;

      oOption.innerText = smeValue;
      oOption.text = smeValue;
      oOption.value = smeValue;
      tbl.deleteRow( rowElem.rowIndex )
      selectElem.disabled = false;
  }


      function addSmeRow(){
//      selectElem.options[selectElem.selectedIndex] = null;
        var selectElem = document.getElementById('smeSelect');
        if( selectElem.value!=null && selectElem.value != ""){
//          document.getElementById('newSme').value = "";
//          var mask = opForm.all.newDstMask.value;
//          var sme = document.getElementById("newSme").value;
          var sme = selectElem.options[selectElem.selectedIndex].value;
          selectElem.options[selectElem.selectedIndex] = null;
          var tbl = document.getElementById("smeTable");

          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);

          newCell = document.createElement("td");
<%--          newCell.innerHTML = '<img src="content/images/mask.gif">';--%>
//          newCell.setAttribute( 'width', '154');
//          newCell.innerHTML = '<input type=hidden name="selected_sme_' + sme + '">' + '<input id="smeSrc_' + sme + '" type=hidden name="smeSubjs" value="' + sme +'">';
          newCell.innerHTML = '<input type=hidden id="selected_sme_' + sme  + '" name="selected_sme_' + sme + '" value="' + sme + '">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
//          newCell.innerHTML = sme + '<input type=hidden name="selected_sme_' + sme + '">';
            newCell.innerHTML = sme;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSmeRow(\'' + sme + '\', \'' + newRow.id + '\')" style="cursor: pointer;">';
          newRow.appendChild(newCell);
          if (selectElem.options.length == 0)
              selectElem.disabled = true;
       }else{
            alert( "Empty ID" );
       }
    }
    </script>
            <br>
<%--                <c:set var="sps" value="${fn:join(bean.spIds, ',')}"/>--%>
                <sm-ep:properties title="metacenter.edit.properties">
                    <br>
                    <sm-ep:txt title="metaeps.edit.txt.id" name="id" maxlength="15" validation=""/>
                    <sm-ep:list title="metaeps.edit.txt.policy" name="policy"
                                values="${fn:join(bean.policyTypes,',')}"
                                valueTitles="${fn:join(bean.policyTitles,',')}"/>
                    <sm-ep:check title="centers.edit.check.enabled.title" head="metaep.edit.check.persistent.head" name="enabled"/>
         <tr>
                <td nowrap valign="top" width="135" ><fmt:message>${fn:escapeXml("metacenter.edit.centerid")}</fmt:message></td>
                <td>
                    <table cellpadding="0" border=0 id=smeTable>
                    <tr>
                        <td></td>
                        <td width="229" >
                            <select id=smeSelect onchange="" name=smeSelect >
<%--                        <option value="">select services</option>--%>
                            <c:forEach items="${bean.availableSmes}" var="i">
                                <option id="${fn:escapeXml(i)}" value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                            </c:forEach>
                        </select>
                        </td>
                        <td><img src="content/images/but_add.gif" onclick="addSmeRow();" style="cursor:pointer;"></td>
                    </tr>
                  <c:forEach items="${bean.selectedSmes}" var="i">
                    <c:set var="sme" value="${fn:escapeXml(i)}"/>
    <%--                <tr class="row${rowN%2}" id="smeRow_${sme}">--%>
                    <tr class="row${rowN%2}" id="smeRow_${sme}">
                      <td>
    <%--                    <img src="content/images/subject.gif">--%>
                        <input id="selected_sme_${sme}" type=hidden name="selected_sme_${sme}" value="${sme}">
<%--                        <input id=smeSrc type=hidden name=smeSubjs value="${sme}">--%>
                      </td>
                      <td>${sme}</td>
                      <td><img src="content/images/but_del.gif" onClick="removeSmeRow( '${sme}', 'smeRow_${sme}');" style="cursor: pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                  </c:forEach>

                    </table>
                </td>
            </tr>
                </sm-ep:properties>
    </jsp:body>
</sm:page>