var ElementObserver = new Class({
  options: {
    elementId : 'id',
    field : 'id'
  },

  initialize : function(options) {
    this.setOptions(options);
  },

  update: function(data) {
    document.getElementById(this.options.elementId).innerHTML = data;
  }
});

ElementObserver.implement(new Options);


var StringTableDataSource = XHR.extend({
  options: {
    url : '',
    onComplete : Class.empty
  },

  initialize: function(options){
    this.addEvent('onSuccess', this.onComplete);
    this.setOptions(options);
    this.parent();
    this.observers = new Array();
  },

  addObserver : function(observer) {
    this.observers[this.observers.length] = observer;
  },

  onComplete : function() {
    var txt = new String(this.response.text);
    var i = txt.indexOf("\n");

    if (i > 0) {

      var services = new Array();
      services = txt.substring(0, i).split(', ');
      var statuses = new Array();
      statuses = txt.substring(i, txt.length).split(', ');

      var observer;
      var element;

      for (var j = 0; j < this.observers.length; j++) {
        observer = this.observers[j];
        for (var k = 0; k < services.length; k++) {
          if (services[k] == observer.options.field) {
            observer.update(statuses[k]);
            break;
          }
        }
      }
    }
  },

  update : function() {
    this.send(this.options.url, '');
  },

  hasObservers : function() {
    return this.observers.length > 0;
  }
});

StringTableDataSource.implement(new Options);