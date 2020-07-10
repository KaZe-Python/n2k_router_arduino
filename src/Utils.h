#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

void debug_print(const char *fmt, ...);
void debug_println(const char *fmt);
int getDaysSince1970(int y, int m, int d);
char* replace(char const * const original, char const * const pattern, char const * const replacement, bool first = false);
bool array_contains(int test, int* int_set, int sz);

struct statistics {
  uint gps_fix = 0;
  uint valid_rmc = 0;
  uint valid_gsa = 0;
  uint invalid_rmc = 0;
  uint invalid_gsa = 0;
  ulong udp_sent = 0;
  ulong udp_failed = 0;
  ulong can_sent = 0;
  ulong can_failed = 0;
  ulong can_received = 0;
};

struct configuration {
  bool use_gps = false;
  bool use_bmp280 = true;
  bool use_dht11 = true;
  bool send_time = false;
};

struct sat {
    int sat_id;
    int elev;
    int az;
    int db;
    int status;
};

struct GSA {
    short valid;
    int nSat;
    int fix;
    int sats[12];
    float hdop;
    float vdop;
    float pdop;
};

struct RMC {
    short valid;
    float lat;
    float lon;

    short y;
    short M;
    short d;
    short h;
    short m;
    short s;
    short ms;

    float cog;
    float sog;
};

struct data {
  RMC rmc;
  GSA gsa;
  double pressure;
  double humidity;
  double temperature;
  double temperature_el;
  double latitude;
  char latitude_NS = 'N';
  double longitude;
  char longitude_EW = 'E';
};

#endif
