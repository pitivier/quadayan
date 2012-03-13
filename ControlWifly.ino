/*
 *  Wifly shield server
 *  
 */
 
#include <Wire.h>

#include "WiFly.h"
#include "Credentials.h"
#include "Command.h"
#include "Ping.h"
#include "DOF.h"
#include "Configuration.h"




unsigned long timer;

Server server(9999);

void setup() {
  pinMode(RED_PIN,OUTPUT);
  pinMode(YELLOW_PIN,OUTPUT);

  digitalWrite(RED_PIN,HIGH);
  
  
  /*** GYROS ***/
  
  Wire.begin();

  initializeGyro();
  calibrateGyro();
  
  initializeAccel();
  computeAccelBias();
  
  timer = millis();
  
  /*** FIN GYROS ***/

  WiFly.begin();

  if (!WiFly.join(ssid, passphrase)) {
    while (1) {
      // Hang on failure.
    }
  }

  server.begin();
  
  // server ready
  digitalWrite(RED_PIN,LOW);
}

void loop() {
  Client client = server.available();
  if (client) {
    client.println("connected \n");
    client.flush();
    digitalWrite(RED_PIN,HIGH);

    boolean current_line_is_blank = true;
    while (client.connected()) {
       
        char cmd;
        while(client.available()){
         
           cmd = client.read();
           
           switch(cmd){
             case CMD_PING :   getDistance(client);
                               break;
             
             case 'c' :        digitalWrite(YELLOW_PIN,HIGH);
                               client.print("received : ");
                               client.print(cmd);
                               client.println("\n");
                               client.flush();
                               break;
                               
             case 'd':         digitalWrite(YELLOW_PIN,LOW);
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
                               
             case CMD_SENSOR_STICK_GYROS:  timer = millis();
                                           measureGyro();
                                            
                                           //client.print("Roll: ");
                                           client.print(degrees(gyroRate[XAXIS]));
                                           //client.print(" Pitch: ");
                                           client.print(";");
                                           client.print(degrees(gyroRate[YAXIS]));
                                           //client.print(" Yaw: ");
                                           client.print(";");
                                           client.print(degrees(gyroRate[ZAXIS]));
                                           //client.print(" Heading: ");
                                           client.print(";");
                                           client.print(degrees(gyroHeading));
                                           client.println();
                                           break;
                                           
             case CMD_SENSOR_STICK_ACCEL:  timer = millis();
                                           measureAccel();
                                          
                                           client.print("Roll: ");
                                           client.print(meterPerSecSec[XAXIS]);
                                           client.print(" Pitch: ");
                                           client.print(meterPerSecSec[YAXIS]);
                                           client.print(" Yaw: ");
                                           client.print(meterPerSecSec[ZAXIS]);
                                           client.println();
                                           break;
                             
             case CMD_INIT_SENSOR_STICK : calibrateGyro();
                                          timer = millis();
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
    digitalWrite(YELLOW_PIN,LOW);
    digitalWrite(RED_PIN,LOW);
  }
}




