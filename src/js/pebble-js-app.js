
var localStorage = window.localStorage;
var example = 1;

var importjQuery = function() {
    var script = document.createElement('script');
    script.src = 'http://code.jquery.com/jquery-latest.min.js';
    script.type = 'text/javascript';
    document.getElementsByTagName('head')[0].appendChild(script);
};

Pebble.addEventListener("ready",
  function(e) {
    importjQuery();
    console.log("Pebble JS Ready!");
  }
);

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL('https://www.billmytime.net/pebble/connect');
});

Pebble.addEventListener("webviewclosed", function(e) {
  var options = JSON.parse(decodeURIComponent(e.response));
	localStorage.setItem('apiuser', options.apiuser);
	localStorage.setItem('apikey', options.apikey);
});


Pebble.addEventListener("appmessage", function(e){
  var action = e.payload[0];
  var returnData = {};
  if (action == "getTasks") {
    if (example) {
      // sample code, just provides a single task back on request currently
      // prep a sample object to send
      returnData = {'0': 't', '1': 'Making Awesome', '2': 'Submitting Awesome', '3': 'Sharing Awesome'};
    } else {
      var projectID = localStorage.getItem("currentproject");
      $.getJSON('https://www.billmytime.net/ajax/get-task-list/' + projectID, function(data) {

        });
    }
    localStorage.setItem("tasklist", returnData);
  } else if (action == "getClients") {
		if (example) {
			returnData = {'0': 'c', '1': 'Acme Inc', '2': 'Pebble Technology', '3': 'Bill my Time'}
		}
	} else if (action == "getProjects") {
		if (example) {
			returnData = {'0': 'p', '1': 'Build the app', '2': 'Build the site'};
		}
	} else if (action == "selProj") {
		localStorage.setItem('currentproject', e.payload[1]);
	}
	if (returnData) {
		Pebble.sendAppMessage(returnData);
	}
});
