var param_counter = 0;

function removeRow(tbl, rowId) {
//    alert("RR " + tbl + " | " + rowId);
    var tbl = getElementByIdUni(tbl);
    for(var i=0; i<tbl.rows.length; i++){
        if(tbl.rows[i].id==rowId){
            tbl.deleteRow(i);
        }
    }
//    var rowElem = tbl.rows[rowId];
//    tbl.deleteRow(rowElem.rowIndex);
}

function removeParametersRow(tbl, rowId){
    var tbl = getElementByIdUni(tbl);
    for(var i=0; i<tbl.rows.length; i++){
        if(tbl.rows[i].id==rowId){
            tbl.deleteRow(i);
        }
    }
}

function addParameter(nameElem, valueElem){
    var pNameElement = document.getElementById("pName");
    var pValueElement = document.getElementById("pValue");


    pNameElement.style.color = "black";
    pValueElement.style.color = "black";

    pNameElement.style.borderColor = "black";
    pValueElement.style.borderColor = "black";    

    var nameElem = getElementByIdUni( nameElem );
    var valueElem = getElementByIdUni( valueElem );
    var paramValue = valueElem.value;
    var paramName = nameElem.value;
    if (trimStr(nameElem.value).length > 0){
        if (trimStr(valueElem.value).length > 0) {
            if (unicName(paramName)) {
                //console.info("Parameter counter: "+param_counter);
                var inner__counter = param_counter++;
                var tbl = getElementByIdUni("temp_param_tbl");
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "param_" + "_" + (inner__counter);
                var newCell = document.createElement("td");
                var sel_name = "par_name_" + paramName + "_" + inner__counter;
                newCell.innerHTML = "<input id=\"" + sel_name + "\""
                                            + " name=\"parameter."+inner__counter+".name\""
                                            + " type=\"text\""
                                            + " size=\"45\""
                                            + " readonly=\"true\""
                                            + " value=\""+paramName+"\"/>";
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.colSpan = 1;
                var sel_value = "par_value_" + paramName + "_" + inner__counter;
                newCell.innerHTML = "<input id=\"" + sel_value + "\""
                                            + " name=\"parameter."+inner__counter+".value\""
                                            + " type=\"text\" "
                                            + "size=\"45\""
                                            + "style=\"color:black;\" value=\"" + paramValue + "\" readonly=\"true\"/>";
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                        + paramName + " parameter\"  onClick=\"removeRow(\'temp_param_tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
                newRow.appendChild(newCell);

                nameElem.value = "";
                valueElem.value = "";
                nameElem.focus();                
                return true;
            } else {
                validationError(nameElem, "Not unic name.");
            }
        } else {
            validationError(valueElem, "Field is empty.");
        }
    } else {
        validationError(nameElem, "Field is empty.");
    }
    return false;
}

function trimStr(sString) {
    while (sString.substring(0, 1) == ' ') {
        sString = sString.substring(1, sString.length);
    }
    while (sString.substring(sString.length - 1, sString.length) == ' ') {
        sString = sString.substring(0, sString.length - 1);
    }
    return sString;
}

function unicName(pName){
    //console.info("Check parameters name unicity.");
    //console.info("pName: "+pName);

    var parameters = document.getElementsByTagName("input");
    
    var l = parameters.length;
    //console.info("Parameters number: " + l);
    var parameter;
    for(var i=0; i<l; i++){
        parameter = parameters[i];
        //console.info("Parameter: " + parameter.name);
        if (parameter.name.indexOf("parameter.") == 0){
            //console.info("Parameter: " + parameter.value);
            if (parameter.value == pName) {
                //console.info("This parameters name already is used.");
                return false;
            } {
                //console.info(parameter.value + " != " + pName);
            }
        }
    }
    return true;
}

