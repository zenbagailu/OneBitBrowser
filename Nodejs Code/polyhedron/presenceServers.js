
var webserver = require('./webserver');
var WebSocketServer = require('ws').Server;
var net = require('net'); //for TCP Sockets




//Specific for servo and sensor interaction (through TCP Sockets)
//---------------------------------------------------------------------
var motorSocket=null;
var sensorSocket=null;


// var deactivateInterval=20000; //for testing...20 seconds
// var activateInterval=deactivateInterval/4; //it will be smaller than the deactivation time to make sense

//Realistic times
var deactivateInterval=5*60*1000; //5 minutes in milliseconds 
var activateInterval=5*60*1000; //5 minutes in milliseconds 

//Demo times
// var deactivateInterval=30*1000; //30 seconds minutes in milliseconds 
// var activateInterval=30*1000; //30 seconds in milliseconds 

if(deactivateInterval<activateInterval){
    console.log('deactiveInterval needs to be smaller than activateInterval! (it is never going to work).');
} 

var lastActivityTime= 0; // 01 January, 1970 UTC...
var firstActivityTime= 0; // 01 January, 1970 UTC...

var activityCount=0; //we use it to know if it is the first time (if motor is down)
var motorState=false;

function logDate(){
    var date = new Date();
    console.log('Date: '+ date.toLocaleDateString() +' ' + date.toLocaleTimeString());
}

function setMotorState(state){

    if(motorState==state){ //do nothing if it is the same
        return;
    }

    motorState=state;

    if(motorSocket){
        motorSocket.write(state? 'up':'down');
    }

    console.log('motor set to '+ (state? 'up':'down') );
    logDate();
}


function processSocketData(socket, data){

	if(data=='motorcr'){ 
        if(motorSocket){ //end if it has connection has been restablished...
            //motorSocket.end();
        }
		motorSocket=socket;
		motorSocket.setKeepAlive(true,5); //so it does not close...
        console.log('motor found. Sending response.');

        motorSocket.write('hello motor', function(){ //write first hello motor, wait 50 ms and set it
            setTimeout(function (){
                //now set it up or down depending og activityCount
                motorSocket.write(motorState? 'up':'down');
                console.log('motor set to '+ (motorState? 'up':'down') );
                logDate();

            }, 5000); //5 seconds

        });


    }else if(data=='sensorcr'){
        if(sensorSocket){ //end if it has connection has been restablished...
            //sensorSocket.end();
        }
    	sensorSocket=socket;
    	sensorSocket.setKeepAlive(true,5); //so it does not close...
    	console.log('sensor found. Sending response.');
        sensorSocket.write('hello sensor.');

     }

    //sometimes the Arduino sends
    //two consecutive "activity" messages concatenated...
    else if(data.toString().substring(0,8)=='activity'){//sent by sensorSocket...

       //process.stdout.write("*"); //best way of doing output...

       if(activityCount==0){ //if first time
        firstActivityTime=lastActivityTime=(new Date()).getTime(); 
        console.log('first Activity registered');
        logDate();
       }
       else{
        lastActivityTime=(new Date()).getTime(); 
        //console.log('last activity time only logged');
       }

        ++activityCount;
    }


    //sent by motor socket. Since it is only listening, the only way of verifying that the 
    //socket is functioning is to send an arbitrary test message (with the string test, in this case)
    //and see if it is actually transmitted. 
    else if(data=='test'){ 
        console.log('test received.'); 
        logDate();
    }
    else{
    	console.log('unrecognised message recived through TCP socket: '+ data);
        //socket.end();
    }
}

//Interval function to check activity and set the activityCount
var interval=1000; //ms 
setInterval(function() {

    // only for terminal debug
    // debugDate=new Date(lastActivityTime);
    // process.stdout.write('Date: '+ debugDate.toLocaleDateString() +' ' + debugDate.toLocaleTimeString() + '\r');


    if(!motorState){ //if it is off, or down...
        
        if(lastActivityTime-firstActivityTime > activateInterval){ //if there has been activity for a while...
            setMotorState(true);
        }

    }else{ //if it is on, or up
    
        if((new Date()).getTime()-lastActivityTime > deactivateInterval){
            firstActivityTime=lastActivityTime;
            activityCount=0;
            setMotorState(false);
        }
    }

}, interval);

//this is for debugging... it checks every specified time how old the lastActivityTime was,
//and prints it if it was less than the interval old... We check then if the sensor is up...
var sensorDebugInterval=120000;// 2 minutes

setInterval(function() {

    debugDate=new Date();
    //console.log(debugDate.toLocaleDateString() +' ' + debugDate.toLocaleTimeString());

    console.log((sensorSocket ? '[*]': '[ ]') 
                +((debugDate.getTime()-lastActivityTime)< sensorDebugInterval ? '(*)': '( )') 
                + ' '
                +(motorSocket ? '[*]': '[ ]')
                +(motorState ? '(u)':'(d)')
                +'      '
                +'date: '
                +debugDate.toLocaleDateString() +' ' + debugDate.toLocaleTimeString()
                );

}, sensorDebugInterval);


//---------------------------------------------------------------------



//socket server for the TCP sockets
//---------------------------------------------------------------------
var socketServerPort = 8080;
//var ss = net.createServer({allowHalfOpen: true});
var ss = net.createServer();
ss.listen(socketServerPort);


ss.on('connection', function(sock) {
    console.log('CONNECTED: ' + sock.remoteAddress +':'+ sock.remotePort);
    logDate(); 


    sock.on('data', function(data) {
        //console.log('message: '+ data);
        processSocketData(this, data);
        
    });

    sock.on('end', function() {

        if(this==motorSocket){
            console.log('FIN Packet sent by other end of motorSocket'); 
        }
        else if(this==sensorSocket){
            console.log('FIN Packet sent by other end of sensorSocket');

        }else{
             console.log('FIN Packet sent by end of unknown');
        }

         

        logDate();      
    });

    sock.on('error', function(error) {

        if(this==motorSocket){
            console.log('error in the motorSocket: ' + error);  
            motorSocket.end();
            motorSocket=null;
        }
        else if(this==sensorSocket){
            console.log('error in the sensorSocket: ' + error); 
            sensorSocket.end();
            sensorSocket=null; 

        }else{
             console.log('error in an unknown TCP socket: ' + error); 
        }
        
        logDate();     
    });

    sock.on('timeout', function() {
        console.log('the TCP socket timed out ');   
        logDate();   
    });

    sock.on('close', function(had_error) {
         if(this==motorSocket){
             console.log('the motorSocket closed' + (had_error? '     with an error': ' without error') ); 
        }
        else if(this==sensorSocket){
             console.log('the sensorSocket closed' + (had_error? '     with an error': ' without error') );  

        }else{
             console.log('an unknown TCP socket closed' + (had_error? '     with an error': ' without error') ); 
        }

        
    });

});

ss.on('listening', function() {
    console.log('Socket server listening on ' + ss.address().address +':'+ ss.address().port);
});

ss.on('close', function() {
    console.log('Socket server closed.');
});

ss.on('error', function(error ) {
    console.log('Error in socket server: ' + error);
});


//---------------------------------------------------------------------



// //webserver for the webpage interface
// //---------------------------------------------------------------------
// var webServerPort=9123;
// //__dirname is the directory where this script resides. If avoided node can only be launch from
// //this directory, or the script won't find index.hrml
// webserver.singlePageWebServer(__dirname+"/index.html", webServerPort); 
// console.log('Web server listenining on port ' + webServerPort);
// //---------------------------------------------------------------------


// //websocket server (for communication with the html served by the webserver, for example)
// //---------------------------------------------------------------------
// var wss = new WebSocketServer({port: 8001});
// var currentWebSocket=null;


// wss.on('connection', function(ws) {

// 	currentWebSocket=ws;

// 	console.log('Websocket connected.');
//     logDate(); 

// 	ws.on('close', function() {
// 		currentWebSocket=null;
// 		console.log('Websocket disconnected.');

// 	});

// 	ws.on('message', function(message) {
//         console.log('message received through webSocket: ' + message);
//         ws.send(''+ message + ' received by server', function(error) {
//         	if(error!=null){
//         		console.log('error: '+ error);
//         	}
//         });

//          //This is for sending data through the tcp sockets
//         if(message=='activity'){
//             console.log('simulated activity from webpage.');

//             if(activityCount==0){
//                 firstActivityTime=lastActivityTime=(new Date()).getTime(); 
//             }
//             else{
//                 lastActivityTime=(new Date()).getTime();
//             }

//             ++activityCount;
//         }
         

//         else{
//         	console.log('unrecognised message recived through webserver: '+message);
//         }
//     });
    
// });

// //---------------------------------------------------------------------


//THIS IS AN ALTERNATIVE WEBSERVER FOR DEMOS


//---------------------------------------------------------------------
var webServerPort=9123;
//__dirname is the directory where this script resides. If avoided node can only be launch from
//this directory, or the script won't find demo.hrml
webserver.singlePageWebServer(__dirname+"/demo.html", webServerPort); 
console.log('Web server listenining on port ' + webServerPort);
//---------------------------------------------------------------------


//websocket server (for communication with the html served by the webserver, for example)
//---------------------------------------------------------------------
var wss = new WebSocketServer({port: 8001});
var currentWebSocket=null;


//---------------------------------------------------------------------


wss.on('connection', function(ws) {

    currentWebSocket=ws;

    console.log('Websocket connected.');

    ws.on('close', function() {
        currentWebSocket=null;
        clearInterval(intervalF); //stop the interval function associated with this socket
        console.log('Websocket disconnected.');

    });

    ws.on('message', function(message) {
        console.log('message received: ' + message);
        ws.send(''+ message + ' received by server', function(error) {
            if(error!=null){
                console.log('error: '+ error);
            }
        });

        //This is for sending data through the tcp sockets
        if(message=='up'){
            console.log('setting motor up from webpage.');
            if(motorSocket!=null){
                motorSocket.write('up');
            }else{
                console.log('no motor connected.');
            }
            
        } else if(message=='down'){
            console.log('setting motor down from webpage.');
            if(motorSocket!=null){
                motorSocket.write('down');
            }else{
                console.log('no motor connected.');
            }
        }else{
            console.log('unrecognised message recived through webserver: '+message);
        }
    });

    //A timer function for updating the sensor state
    var webUpdateInterval=1000; //every second

    var intervalF=setInterval(function() {
        wuDate = new Date();
        ws.send((wuDate.getTime()-lastActivityTime)< sensorDebugInterval ? 'sensor on' : 'sensor off' );
    }, webUpdateInterval);
 
});


//---------------------------------------------------------------------

//Grateful ShutDown
process.on('SIGTERM', function () {

    

    if(motorSocket){
        console.log('motorSocket closing process started.');
        motorSocket.setKeepAlive(false);
        motorSocket.end();
        //motorSocket.unref(); //trying if this actually prevents sockets not ending...
    }

    if(sensorSocket){
        console.log('sensorSocket closing process started.');
        sensorSocket.setKeepAlive(false);
        sensorSocket.end();
        //sensorSocket.unref();  //trying if this actually prevents sockets not ending...
    }

    //stop accepting connections (it will emit  a 'close' event when all connections are closed )
    //the callback we pass will execute at the 'close' event
    ss.close(function(){
        console.log('Graceful shutdown executed. Exiting now.');
        process.exit(0);
    });


    // setTimeout(function(){
    //     console.log('Graceful shutdown executed. Exiting now.');
    //     process.exit(0);
    // }, 10000); //wait for ten seconds while the sockets close (and process messsages...)

 
});


