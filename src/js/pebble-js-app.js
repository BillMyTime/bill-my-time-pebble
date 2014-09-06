
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
	if (!example) {
		var apiuser = localStorage.getItem('apiuser');
		var apikey = localStorage.setItem('apikey');
	}

	switch (action) {
		case "getTasks":
			console.log("received request for task list");
			if (example) {
				// sample code, just provides a single task back on request currently
				// prep a sample object to send
				returnData = {'0': 't', '1': 'Making Awesome', '2': 'Submitting Awesome', '3': 'Sharing Awesome'};
			} else {
				var projectID = localStorage.getItem("currentproject");
				$.getJSON('https://www.billmytime.net/ajax/get-task-list/' + projectID + '/' + apiuser + '/' + apikey, function(data) {
						returnData[0] = 't';
						var i = 1;
						var menustore = {};
						$.each(data, function(index, obj) {
							returnData[i] = obj.title;
							menustore[i] = {
								title:obj.title,
								taskid: obj.id,
								};
								i++;
							});
						localStorage.setItem("tasklist", JSON.stringify(menustore));
					});

			}
			break;
		case "getClients":
			console.log("received request for client list");
			if (example) {
				returnData = {'0': 'c', '1': 'Acme Inc', '2': 'Pebble Technology', '3': 'Bill my Time'}
			} else {
				$.getJSON('https://www.billmytime.net/ajax/get-client-list/' + apiuser + '/' + apikey, function(data) {
					returnData[0] = 'c';
					var i = 1;
					var menustore = {};
					$.each(data, function(index, obj) {
						returnData[i] = obj.title;
						menustore[i] = {
							title:obj.title,
							clientid: obj.id,
						};
						i++;
					});
					localStorage.setItem("clientlist", JSON.stringify(menustore));
				});
			}
			break;
		case "getProjects":
			console.log("received request for project list");
			if (example) {
				returnData = {'0': 'p', '1': 'Build the app', '2': 'Build the site'};
			} else {

			}
			break;
		case "selProj":
			console.log("received request to set current project");
			localStorage.setItem('currentproject', e.payload[1]);
		default:
			console.log("case is a bitch");

	}
	if (returnData) {
		Pebble.sendAppMessage(returnData);
	} else {
		Pebble.showSimpleNotificationOnPebble("No data found", "Could not match your request");
	}
});
