#ifndef N2K_H
#define N2K_H

#include <N2kMessages.h>
#include "NMEA.h"

class N2K {

    public:
        void sendCOGSOG(GSA& gsa, RMC& rmc);
        void sendTime();
        void sendTime(GSA& gsa, RMC& rmc);
        void sendLocalTime(GSA& gsa, RMC& rmc);
        void sendPosition(GSA& gsa, RMC& rmc);

        int get_sent() { return g_pos_sent; }
        int get_sent_fail() { return g_pos_sent_fail; }
        void reset_counters() {
            g_pos_sent = 0;
            g_pos_sent_fail = 0;
        }

        void setup(void (*_MsgHandler)(const tN2kMsg &N2kMsg));

        void loop();

    private:
        uint g_pos_sent;
        uint g_pos_sent_fail;
        void (*handler)(const tN2kMsg &N2kMsg);
        void handle_message(const tN2kMsg &N2kMsg);
};

#endif