#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>

#define VERSION "1.0.0"  // Versão atual do firmware

/* URLs do Firebase Storage */
#define VERSION_URL "https://firebasestorage.googleapis.com/v0/b/eduardo-d28ce.appspot.com/o/preMax%2FOTA%2Fversion.txt?alt=media"
#define FIRMWARE_URL "https://firebasestorage.googleapis.com/v0/b/eduardo-d28ce.appspot.com/o/preMax%2FOTA%2Ffirmware.bin?alt=media"

class updateOTA {
    public:
        updateOTA(void);
        void updateFirmware();
        void checkForUpdate();
};

#endif