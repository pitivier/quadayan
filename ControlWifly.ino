/*
 * Web Server
 *
 * (Based on Ethernet's WebServer Example)
 *
 * A simple web server that shows the value of the analog input pins.
 */
#include <Wire.h>

#include "WiFly.h"
#include "Credentials.h"
#include "Command.h"
#include "Ping.h"
#include "Configuration.h"
#include "DOF.h"


unsigned long timer;

Server server(9999);

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(yellowPin,OUTPUT);

  digitalWrite(ledPin,HIGH);
  
  
  /*** GYROS ***/
  
  Wire.begin();

  initializeGyro();
  calibrateGyro();
  timer = millis();
  
  /*** FIN GYROS ***/

  WiFly.begin();

  if (!WiFly.join(ssid, passphrase)) {
    while (1) {
      // Hang on failure.
    }
  }

  server.begin();
  digitalWrite(ledPin,LOW);
}

void loop() {
  Client client = server.available();
  if (client) {
    client.println("connected \n");
    client.flush();
    digitalWrite(ledPin,HIGH);

    boolean current_line_is_blank = true;
    while (client.connected()) {
       
        char cmd;
        while(client.available()){
         
           cmd = client.read();
           
           switch(cmd){
             case CMD_PING :   getDistance(client);
                               break;
             
             case 'c' :        digitalWrite(yellowPin,HIGH);
                               client.print("received : ");
                               client.print(cmd);
                               client.println("\n");
                               client.flush();
                               break;
                               
             case 'd':         digitalWrite(yellowPin,LOW);
                               client.print("received : ");
                               client.print(cmd);
                               client.println("\n");
                               client.flush();
                               
             case 'e':         for (int i = 0; i < 6; i++) {
                                 if(i > 0){
                                   client.print(",");
                                 }
                                 client.print(analogRead(i));  
                               }
                               
             case 's':         timer = millis();
                               measureGyro();
                                
                               client.print("Roll: ");
                               client.print(degrees(gyroRate[XAXIS]));
                               client.print(" Pitch: ");
                               client.print(degrees(gyroRate[YAXIS]));
                               client.print(" Yaw: ");
                               client.print(degrees(gyroRate[ZAXIS]));
                               client.print(" Heading: ");
                               client.print(degrees(gyroHeading));
                               client.println();
                               break;
                             
             default:          break;
           }
           client.println("\n");
           client.flush();

           delay(100);
        }
     
    }
    delay(100);
    client.stop();
    digitalWrite(yellowPin,LOW);
    digitalWrite(ledPin,LOW);
  }
}




