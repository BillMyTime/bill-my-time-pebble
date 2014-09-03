
var localStorage = window.localStorage;

Pebble.addEventListener("ready",
  function(e) {
    console.log("JavaScript app ready and running!");
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
	// sample code, just provides a single task back on request currently
	var action = e.payload[0];
	var data = {};
	if (action == "getTasks") {
		// prep a sample object to send
		data = {'0': 't', '1': 'Making Awesome Happen', '2': 'Submitting Awesome'};
	}
	Pebble.sendAppMessage(data);

});
