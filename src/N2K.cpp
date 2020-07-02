#include <Arduino.h>
#define USE_N2K_CAN USE_N2K_MCP_CAN
#define N2k_SPI_CS_PIN 5
#define N2k_CAN_INT_PIN 0xff
#define USE_MCP_CAN_CLOCK_SET 8
#include <NMEA2000_CAN.h>
#include <time.h>
#include "N2K.h"
#include "Utils.h"

ulong* _can_received;
void (*_handler)(const tN2kMsg &N2kMsg);

void private_message_handler(const tN2kMsg &N2kMsg) {
    (*_can_received)++;
    _handler(N2kMsg);
}

void N2K::loop() {
    NMEA2000.ParseMessages();
}

void N2K::setup(void (*_MsgHandler)(const tN2kMsg &N2kMsg), statistics* s) {
    stats = s;
    _can_received = &(s->can_received);
    _handler = _MsgHandler;
    debug_println("Initializing N2K");
    NMEA2000.SetN2kCANSendFrameBufSize(250);
    debug_println("Initializing N2K Product Info");
    NMEA2000.SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "ABN2k",  // Manufacturer's Model ID
                                 "1.0.2.25 (2019-07-07)",  // Manufacturer's Software version code
                                 "1.0.2.0 (2019-07-07)" // Manufacturer's Model version
                                 );
    debug_println("Initializing N2K Device Info");
    NMEA2000.SetDeviceInformation(1, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
    debug_println("Initializing N2K mode");
    NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 22);
    NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
    debug_println("Initializing N2K Port & Handlers");
    NMEA2000.SetMsgHandler(private_message_handler);
    bool initialized = NMEA2000.Open();
    debug_print("Initializing N2K %s\n", initialized?"OK":"KO");
}

bool N2K::send_msg(const tN2kMsg &N2kMsg) {
    if (NMEA2000.SendMsg(N2kMsg)) {
        _handler(N2kMsg);
        stats->can_sent++;
        return true;
    } else {
        stats->can_failed++;
        return false;
    }
}


bool N2K::sendCOGSOG(GSA& gsa, RMC& rmc) {
    if (gsa.valid && rmc.valid && gsa.fix >= 2) {
        if (isnan(rmc.sog) && isnan(rmc.cog)) return false;
        else {
            tN2kMsg N2kMsg;
            SetN2kCOGSOGRapid(N2kMsg, 1, N2khr_true, DegToRad(isnan(rmc.cog)?0.0:rmc.cog), rmc.sog * 1852.0 / 3600);
            return send_msg(N2kMsg);
        }
    }
    return false;
}

bool N2K::sendTime(GSA& gsa, RMC& rmc) {
    if (gsa.valid && rmc.valid && gsa.fix >= 2) {
        tN2kMsg N2kMsg;
        int days_since_1970 = getDaysSince1970(rmc.y, rmc.M, rmc.d);
        double second_since_midnight = rmc.h * 60 * 60 + rmc.m * 60 + rmc.s + rmc.ms / 1000.0;
        SetN2kSystemTime(N2kMsg, 1, days_since_1970, second_since_midnight);
        return send_msg(N2kMsg);
    }
    return false;
}

bool N2K::sendLocalTime(GSA& gsa, RMC& rmc) {
    if (gsa.valid && rmc.valid && gsa.fix >= 2)  {
        tN2kMsg N2kMsg;
        int days_since_1970 = getDaysSince1970(rmc.y, rmc.M, rmc.d);
        double second_since_midnight = rmc.h * 60 * 60 + rmc.m * 60 + rmc.s + rmc.ms / 1000.0;
        SetN2kLocalOffset(N2kMsg, days_since_1970, second_since_midnight, 0);
        return send_msg(N2kMsg);
    }
    return false;
}

bool N2K::sendTime(time_t _now) {
    tm* t = gmtime(&_now);
    int days_since_1970 = getDaysSince1970(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    double second_since_midnight = t->tm_hour * 60 * 60 + t->tm_min * 60 + t->tm_sec;
    tN2kMsg N2kMsg;
    SetN2kSystemTime(N2kMsg, 1, days_since_1970, second_since_midnight);
    return send_msg(N2kMsg);
}

bool N2K::sendPosition(GSA& gsa, RMC& rmc) {
    if (gsa.valid && rmc.valid && gsa.fix >= 2) {
        tN2kMsg N2kMsg;
        SetN2kPGN129025(N2kMsg, rmc.lat, rmc.lon);
        return send_msg(N2kMsg);
    }
    return false;
}

bool N2K::sendEnvironment(const float pressurePA, const float humidity, const float temperatureC) {
    tN2kMsg N2kMsg;
    SetN2kEnvironmentalParameters(N2kMsg, 0, 
        N2kts_MainCabinTemperature, CToKelvin(temperatureC), 
        N2khs_InsideHumidity, humidity, 
        pressurePA);
    return send_msg(N2kMsg);
}
