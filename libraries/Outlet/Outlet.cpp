#include "Outlet.h"
#include "Arduino.h"

using std::string;

#define periods 5

Outlet::Outlet(){
    _status = false;
    _name = "Outlet";
    _timer = 0;
    _power = new int* [periods];
    _power[0] = new int [60];
    _power[1] = new int [24];
    _power[2] = new int [30];
    _power[3] = new int [12];
    _power[4] = new int [10];
}

bool Outlet::getStatus(){
    return _status;
}
string Outlet::getName(){
    return _name;
}
bool Outlet::getTimerUse(){
    return _timerUse;
}
char Outlet::getTimer(){
    return _timer;
}
int* Outlet::getPower(int period){
    return _power[period];
}

void Outlet::setStatus(bool status){
    _status = status;
}
void Outlet::setName(string name){
    _name = name;
}
void Outlet::setTimerUse(bool timerUse){
    _timerUse = timerUse;
}
void Outlet::setTimer(char timer){
    _timer = timer;
}
void Outlet::setPower(int period, int address, int power){
    _power[period][address] = power;
}

Outlet::~Outlet(){
    for(int i=0; i<periods; i++){
        delete[] _power[i];
    }
    delete[] _power;
}
