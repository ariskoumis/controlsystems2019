#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <WiFi.h>
#include "Source.h"
#include "Arduino.h"
#include "EEPROM.h"
#include "constants.h"

void initServer(AsyncWebServer* server, ParamsStruct* params) {
    //Create Access Point
    WiFi.softAP("MyESP32AP", "testpassword");
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    
    AsyncEventSource events("/events");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    /* XHR Example.
        - Param "name" is sent from mission control.
        - "name" is copied to the params object
        - params object is then passed to vSayHelloTask - see main.cpp
        - vSayHello task then accesses name directly.

        Note: for ANY parameters you want to use, you must add them to
        the paramsStruct struct located in Source.h first. 
    */


    server->on("/handle_update", HTTP_POST, [=](AsyncWebServerRequest *request){
        const char *vars[8] = {
            "mode", "wheel_1_speed", "wheel_1_heading", "wheel_2_speed", 
            "wheel_2_heading", "wheel_3_speed", "wheel_3_heading", "brake"
        };

        for (int i=0; i<8; i++) {
            if (request->hasArg(vars[i])) {
                if (strcmp(vars[i], "mode")) {
                    strcpy(params->mode, request->arg("mode").c_str());  
                }
                if (strcmp(vars[i], "wheel_1_speed")) {
                    params->wheel_1_speed = request->arg("wheel_1_speed").toInt();    
                }
                if (strcmp(vars[i], "wheel_1_heading")) {
                    params->wheel_1_heading = request->arg("wheel_1_heading").toInt();    
                }
                if (strcmp(vars[i], "wheel_2_speed")) {
                    params->wheel_2_speed = request->arg("wheel_2_speed").toInt();    
                }
                if (strcmp(vars[i], "wheel_2_heading")) {
                    params->wheel_2_heading = request->arg("wheel_2_heading").toInt();    
                }
                if (strcmp(vars[i], "wheel_3_speed")) {
                    params->wheel_3_speed = request->arg("wheel_3_speed").toInt();    
                }
                if (strcmp(vars[i], "wheel_3_heading")) {
                    params->wheel_3_heading = request->arg("wheel_3_heading").toInt();    
                }
                if (strcmp(vars[i], "brake")) {
                    params->brake = (bool) request->arg("brake").toInt();    
                }
            }
            else {
                printf("ERROR. %s doesn't exist", vars[i]);
            }
        }
        
        printf("handle_update endpoint running\n");
        printf("    mode: %s \n", params->mode);
        printf("    wheel_1_speed: %i \n", params->wheel_1_speed);
        printf("    wheel_1_heading: %i \n", params->wheel_1_heading);
        printf("    wheel_2_speed: %i \n", params->wheel_2_speed);
        printf("    wheel_2_heading: %i \n", params->wheel_2_heading);
        printf("    wheel_3_speed: %i \n", params->wheel_3_speed);
        printf("    wheel_3_heading: %i \n", params->wheel_3_heading);
        printf("    brake: %i \n", params->brake);
        printf("\n");

        request->send(200, "text/plain", "Success");
    });
    
    /* SSE Example.
        - SSEs will be used to continuously send data that was
        not necessarily requested by mission control
        (e.g. temperature, something we should send periodically)

        - Once mission control declares the ESPs IP address at a certain 
        endpoint to be an EventSource, the ESP can trigger events on the web
        interface, which the web interface can attach event listeners to
        (similar to how we are attaching event listeners for when we recieve
        XHRs to /update_name above, allowing us to do things when we recieve an 
        XHR).
        - Below's example is an example of sending SSEs when mission control
        declares our ip address and endpoint (e.g. 192.168.4.1/events) to be
        an event source.
        - More info on this concept here: 
            https://developer.mozilla.org/en-US/docs/Web/API/EventSource
    */
    
    events.onConnect([](AsyncEventSourceClient *client) {
      if(client->lastId())
      {
        Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
    });

    //Attach event source to the server.
    server->addHandler(&events);

    //Start server.
    server->begin();
}

bool initEEPROM() {
    bool status = EEPROM.begin(EEPROM_SIZE);
    switch(status)
    {
        case 0:
	    printf("ERROR: EEPROM initialization failure.\n");
	    usleep(1000);
	    break;
	case 1:
	    printf("Successfully initialized EEPROM, size = %d.\n", EEPROM_SIZE);
	    usleep(1000);
	    break;
	default:
	    break;
    }
    return status;    
}

int EEPROMCount(int addr)
{
    int data = EEPROM.read(addr);
    data++;
    EEPROM.write(addr, data);
    EEPROM.commit();
    return data;
}