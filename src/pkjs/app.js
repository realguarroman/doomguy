Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("http://www.azucarconamor.es/pebble/docs/config_doomguy2.html");
  }
);


Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
 
    //Send to Pebble, persist there
		
	var dictionary = {
				"KEY_BEHAVIOUR": configuration.behaviour,
			  "KEY_FONT_COLOR": parseInt(configuration.font_color, 16)
			/*	"KEY_ANIMATE_NUMBERS": configuration.animate_numbers,
				"KEY_BT_CONNECT": configuration.bt_connect,
				"KEY_BT_DISCONNECT": configuration.bt_disconnect */
		};
		
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);


