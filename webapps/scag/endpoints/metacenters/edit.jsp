<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">metaservice.add.title</c:when>
            <c:otherwise>metaservice.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="metaeps.edit.item.mbsave.value" title="metaeps.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="metaeps.edit.item.mbcancel.value" title="metaeps.edit.item.mbcancel.title"
                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <script>
    var global_counter = 0;
  function removeSmeRow( tblId, rowId ){
      tbl = document.getElementById("smeTable");
      var rowElem = tbl.rows[rowId];
      var selectElem = document.getElementById('smeSelect');
      var oOption = document.createElement("OPTION");
//      alert( "SME DELETE:" + rowElem.childNodes[3].innerHTML );
      var smeValue = rowElem.childNodes[3].innerHTML;
      oOption.innerText = smeValue;
      oOption.text = smeValue;
      oOption.value = smeValue;
      selectElem.options.add(oOption);

      var rowElem = tbl.rows[rowId];
//      alert( "REMOVE INDEX:" + rowElem.rowIndex );
      tbl.deleteRow( rowElem.rowIndex )
  }


      function addSmeRow(){
//      alert("add sme='" + document.getElementById("newSme").value +"'");
//      selectElem.options[selectElem.selectedIndex] = null;
        var selectElem = document.getElementById('smeSelect');
//         if( document.getElementById("newSme").value!=null && document.getElementById("newSme").value != ""){
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
          newCell.innerHTML = '<input type=hidden name="selected_sme_' + sme + '">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
//          newCell.innerHTML = sme + '<input type=hidden name="selected_sme_' + sme + '">';
            newCell.innerHTML = sme;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          var img     = document.createElement('IMG');
          img.setAttribute('src', 'content/images/but_del.gif');
          img.setAttribute('style', 'cursor:pointer');
          img.onclick = function(){
              var rowElem = tbl.rows[newRow.id];
              var selectElem = document.getElementById('smeSelect');
              var oOption = document.createElement("OPTION");
              alert( "SME:" + rowElem.childNodes[1].innerHTML );
              var smeValue = rowElem.childNodes[1].innerHTML;
              oOption.innerText = smeValue;
              oOption.text = smeValue;
              oOption.value = smeValue;
              selectElem.options.add(oOption);
//                var rowElem = tbl.rows[newRow.id];
//                alert( "RREMOVE INDEX:" + rowElem.rowIndex );
                tbl.deleteRow( rowElem.rowIndex )
          }
          newCell.appendChild(img);
//          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSmeRow(opForm.all.smeTable, \'' + newRow.id + '\')" style="cursor: hand;">';
          newRow.appendChild(newCell);

//          alert( "Table=" + tbl.rows.length);
//         }else{
//            alert( "Empty SME" );
//         }
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
                            <select id=smeSelect onchange="setNewSme()" name=smeSelect >
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
                        <input type=hidden name="selected_sme_${sme}" value="sme"}">
                      </td>
                      <td>${sme}</td>
                      <td><img src="content/images/but_del.gif" onClick="removeSmeRow( 'smeTable', 'smeRow_${sme}');" style="cursor: pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                  </c:forEach>

                    </table>
                </td>
            </tr>
                </sm-ep:properties>
    </jsp:body>
</sm:page>