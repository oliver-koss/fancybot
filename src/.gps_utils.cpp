#include <Arduino.h>
#include "gps_utils.h"

namespace GPS_Utils {

    void setup()
    {
        xTaskCreate(gps_task, "gps_task", 10000, NULL, 1, NULL);
    }

    void gps_task(void* parameter)
    {
        while(true)
        {
            gps_lock.lock();
    //        gps.encode(gpsSerial.read());

            while (gpsSerial.available() > 0){
                // get the byte data from the GPS
                gps.encode(gpsSerial.read());
            }

            /*
            while (gpsSerial.available() > 0){
                // get the byte data from the GPS
                char gpsData = gpsSerial.read();
                Serial.print(gpsData);
            }
            */

            if(gps.time.age() > 1500 && gps_enabled)
            {
                log_event("hardware", "gps_working", 0);
                gps_enabled = false;
            }

            if(gps.time.age() < 1500 && !gps_enabled)
            {
                log_event("hardware", "gps_working", 1);
                gps_enabled = true;
            }

            if(gps_enabled && !gps_location_valid && gps.location.isValid())
            {
                log_event("hardware", "gps_location_valid", 1);
                gps_location_valid = true;
            }
            if(gps_location_valid && !gps.location.isValid()){
                log_event("hardware", "gps_location_valid", 0);
                gps_location_valid = false;
            }

            if (gps.time.isValid() && !rtc_set && rtc_enabled)
            {
                DateTime dt(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
                rtc.adjust(dt);
            //   rtc.adjust(gps.time.second(), gps.time.minute(), gps.time.hour(), gps.date.day(), gps.date.month(), gps.date.year());
                rtc_set = true;
                log_event("hardware", "rtc_set", 1);
    //            Serial.print("Time set!");
            }
            gps_lock.unlock();

            delay(500);
    //        log_event("test", "test_logging", 1);
        }
    }


}
