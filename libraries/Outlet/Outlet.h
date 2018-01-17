#include "Arduino.h"

#ifndef Outlet_h
#define Outlet_h

using std::string;

Class Outlet
{
    public:
        Outlet();
        bool getStatus();
        string getName();
        bool getTimerUse();
        char getTimer();
        int getPower(int period, int address);

        void setStatus(bool status);
        void setName(string name);
        void setTimerUse(bool timerUse);
        void setTimer(char timer);
        void setPower(int period, int address, int power);

    private:
        bool _status;
        string _name;
        bool _timerUse;
        char _timer;

        int _minutes [];
        int _hours [];
        int _days [];
        int _months [];
        int _years [];
        */
}

#endif
