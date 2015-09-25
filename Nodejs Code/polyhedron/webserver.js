var http = require("http");
var url = require("url"); //to process url in the request
var fs =require("fs");


function singlePageWebServer(htmlPagePath, port){

	function onRequest(request, response) {

		var pathname = url.parse(request.url).pathname;
		console.log("Request for " + pathname + " received.");

		var htmlFile = fs.readFileSync(htmlPagePath); //default options, utf8 encoding, r flag.
		//var htmlFile = fs.readFileSync("index.html"); //default options, utf8 encoding, r flag.

		 response.writeHead(200, {"Content-Type": "text/html"}); 
		 response.write(htmlFile);
		 response.end();
	}

	try{
		http.createServer(onRequest).listen(port);
	}catch(error){
			console.error(error);
			return;
	}
	console.log("Web server has started.");
}

exports.singlePageWebServer = singlePageWebServer;




