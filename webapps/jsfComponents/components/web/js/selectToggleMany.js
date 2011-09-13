
function toggleAll(formName,checkBoxName) {
  if(this.checked==null) this.checked=true;
  var elements = document.forms[formName].elements;
  for(var i=0;i<elements.length;i++) {
    if(elements[i].name==checkBoxName) {
      elements[i].checked = this.checked;
    }
  }
  this.checked = !this.checked;
  return false;
}