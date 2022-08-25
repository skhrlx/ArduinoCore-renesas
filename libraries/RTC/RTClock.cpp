#include "RTClock.h"
#include <Arduino.h>

/* -------------------------------------------------------------------------- */
/*                     HELPER CLASS AlarmMatch                                */
/* -------------------------------------------------------------------------- */

/* ------------------------------ DEFINES ----------------------------------- */
#define MATCH_SECOND_POS    (0)
#define MATCH_MINUTE_POS    (1)
#define MATCH_HOUR_POS      (2)
#define MATCH_DAY_POS       (3)
#define MATCH_MONTH_POS     (4)
#define MATCH_YEAR_POS      (5)
#define MATCH_DAYOFWEEK_POS (6)
/* no more than 32 */

#define IS_MATCH_SET(reg,pos)  ((reg & (1<<pos)) >> pos)  
#define SET_MATCH(reg,pos)     reg |= (1<<pos)
#define RESET_MATCH(reg,pos)   reg &= ~(1<<pos)

/* -------------------------------------------------------------------------- */
AlarmMatch::AlarmMatch() : match{0} { }
AlarmMatch::~AlarmMatch() {}    

void AlarmMatch::addMatchSecond()       { SET_MATCH(match,MATCH_SECOND_POS); }
void AlarmMatch::addMatchMinute()       { SET_MATCH(match,MATCH_MINUTE_POS); }
void AlarmMatch::addMatchHour()         { SET_MATCH(match,MATCH_HOUR_POS); }
void AlarmMatch::addMatchDay()          { SET_MATCH(match,MATCH_DAY_POS); }
void AlarmMatch::addMatchMonth()        { SET_MATCH(match,MATCH_MONTH_POS); }
void AlarmMatch::addMatchYear()         { SET_MATCH(match,MATCH_YEAR_POS); }
void AlarmMatch::addMatchDayOfWeek()    { SET_MATCH(match,MATCH_DAYOFWEEK_POS); }
void AlarmMatch::removeMatchSecond()    { RESET_MATCH(match,MATCH_SECOND_POS); }
void AlarmMatch::removeMatchMinute()    { RESET_MATCH(match,MATCH_MINUTE_POS); }
void AlarmMatch::removeMatchHour()      { RESET_MATCH(match,MATCH_HOUR_POS); }
void AlarmMatch::removeMatchDay()       { RESET_MATCH(match,MATCH_DAY_POS); }
void AlarmMatch::removeMatchMonth()     { RESET_MATCH(match,MATCH_MONTH_POS); }
void AlarmMatch::removeMatchYear()      { RESET_MATCH(match,MATCH_YEAR_POS); }
void AlarmMatch::removeMatchDayOfWeek() { RESET_MATCH(match,MATCH_DAYOFWEEK_POS); }
bool AlarmMatch::isMatchingSecond()     { return (bool)IS_MATCH_SET(match,MATCH_SECOND_POS); }
bool AlarmMatch::isMatchingMinute()     { return (bool)IS_MATCH_SET(match,MATCH_MINUTE_POS);}
bool AlarmMatch::isMatchingHour()       { return (bool)IS_MATCH_SET(match,MATCH_HOUR_POS);}
bool AlarmMatch::isMatchingDay()        { return (bool)IS_MATCH_SET(match,MATCH_DAY_POS);}
bool AlarmMatch::isMatchingMonth()      { return (bool)IS_MATCH_SET(match,MATCH_MONTH_POS);}
bool AlarmMatch::isMatchingYear()       { return (bool)IS_MATCH_SET(match,MATCH_YEAR_POS);}
bool AlarmMatch::isMatchingDayOfWeek()  { return (bool)IS_MATCH_SET(match,MATCH_DAYOFWEEK_POS);}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                        HELPER CLASS RTCTime                                */
/* -------------------------------------------------------------------------- */

#define MIN_DAY_OF_MONTH (1)
#define MAX_DAY_OF_MONTH (31)
#define MIN_HOURS        (0)
#define MAX_HOURS        (23)
#define MIN_MINUTES      (0)
#define MAX_MINUTES      (59)
#define MIN_SECONDS      (0)
#define MAX_SECONDS      (59)

#define TM_YEAR_OFFSET   (1900)

#define NO_SAVE_LIGHT_ACTIVE (0)
#define SAVE_LIGHT_ACTIVE (1)


int isleap(int year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int yday(int year, int month)
{
    static const short int upto[12] =
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int yd;

    yd = upto[month - 1];
    if (month > 2 && isleap(year))
        yd++;
    return yd;
}

int Month2int(Month m) {
    switch(m) {
        case Month::JANUARY: return 1; 
        case Month::FEBRUARY: return 2; 
        case Month::MARCH: return 3; 
        case Month::APRIL: return 4; 
        case Month::MAY: return 5; 
        case Month::JUNE: return 6; 
        case Month::JULY: return 7; 
        case Month::AUGUST: return 8; 
        case Month::SEPTEMBER: return 9; 
        case Month::OCTOBER: return 10; 
        case Month::NOVEMBER: return 11;
        case Month::DECEMBER: return 12; 
        default:         return 1;    
    }
    return 1;    
}


int Month2tm(Month m) {
    switch(m) {
        case Month::JANUARY: return 0; 
        case Month::FEBRUARY: return 1; 
        case Month::MARCH: return 2; 
        case Month::APRIL: return 3; 
        case Month::MAY: return 4; 
        case Month::JUNE: return 5; 
        case Month::JULY: return 6; 
        case Month::AUGUST: return 7; 
        case Month::SEPTEMBER: return 8; 
        case Month::OCTOBER: return 9; 
        case Month::NOVEMBER: return 10;
        case Month::DECEMBER: return 11; 
        default:         return 0;    
    }
    return 0;
}

int SaveLight2tm(SaveLight sl) {
    int rv = (sl == SaveLight::SAVING_TIME_INACTIVE) ? NO_SAVE_LIGHT_ACTIVE : SAVE_LIGHT_ACTIVE;
    return rv;
}

bool SaveLigth2bool(SaveLight sl) {
    bool rv = (sl == SaveLight::SAVING_TIME_INACTIVE) ? false : true;
    return rv;
}

int DayOfWeek2tm(DayOfWeek dow) {
    switch(dow) {
        case DayOfWeek::SUNDAY:    return 0;
        case DayOfWeek::MONDAY:    return 1;
        case DayOfWeek::TUESDAY:   return 2;
        case DayOfWeek::WEDNESDAY: return 3;
        case DayOfWeek::THURSDAY:  return 4;
        case DayOfWeek::FRIDAY:    return 5;
        case DayOfWeek::SATURDAY:  return 6;
        default:                   return 0;   
    }
    return 0;
}

int DayOfWeek2int(DayOfWeek dow, bool sunday_first) {
    if(sunday_first) {
        return DayOfWeek2tm(dow) + 1;
    }
    else {
        switch(dow) {
            case DayOfWeek::SUNDAY:    return 7;
            case DayOfWeek::MONDAY:    return 1;
            case DayOfWeek::TUESDAY:   return 2;
            case DayOfWeek::WEDNESDAY: return 3;
            case DayOfWeek::THURSDAY:  return 4;
            case DayOfWeek::FRIDAY:    return 5;
            case DayOfWeek::SATURDAY:  return 6;
            default:                   return 1; 
        }  
    }
    return 1;
}

Month tm2Month(struct tm &t) {
    switch(t.tm_mon) {
        case 0: return Month::JANUARY;
        case 1: return Month::FEBRUARY;
        case 2: return Month::MARCH;
        case 3: return Month::APRIL;
        case 4: return Month::MAY;
        case 5: return Month::JUNE;
        case 6: return Month::JULY;
        case 7: return Month::AUGUST;
        case 8: return Month::SEPTEMBER;
        case 9: return Month::OCTOBER;
        case 10: return Month::NOVEMBER;
        case 11: return Month::DECEMBER;
        default: return Month::JANUARY;
    }
    return Month::JANUARY;
}

DayOfWeek tm2DayOfWeek(struct tm &t) {
    switch(t.tm_wday) {
        case 0: return DayOfWeek::SUNDAY;
        case 1: return DayOfWeek::MONDAY;
        case 2: return DayOfWeek::TUESDAY;
        case 3: return DayOfWeek::WEDNESDAY;
        case 4: return DayOfWeek::THURSDAY;
        case 5: return DayOfWeek::FRIDAY;
        case 6: return DayOfWeek::SATURDAY;
        default: return DayOfWeek::SUNDAY;
    }
    return DayOfWeek::SUNDAY;
}

SaveLight tm2SaveLight(struct tm &t) {
    return (t.tm_isdst == 0) ?  SaveLight::SAVING_TIME_INACTIVE : SaveLight::SAVING_TIME_ACTIVE;
}

RTCTime::RTCTime(struct tm &t)  {
    setTM(t);
}

RTCTime::RTCTime() : day{1}, 
                     month{Month::JANUARY}, 
                     year{2000}, 
                     hours{0}, 
                     minutes{0}, 
                     seconds{0}, 
                     day_of_week{DayOfWeek::SATURDAY}, 
                     save_light{SaveLight::SAVING_TIME_INACTIVE} { }

RTCTime::RTCTime(int _day, 
                 Month _m, 
                 int _year, 
                 int _hours, 
                 int _minutes, 
                 int _seconds, 
                 DayOfWeek _dof, 
                 SaveLight _sl) : RTCTime() {
    setDayOfMonth(_day);
    setMonthOfYear(_m);
    setYear(_year);
    setHour(_hours);
    setMinute(_minutes);
    setSecond(_seconds);
    setDayOfWeek(_dof);
    setSaveLight(_sl);
}
RTCTime::~RTCTime() { }
    
/* setters */
void RTCTime::setTM(struct tm &t) {
    setDayOfMonth(t.tm_mday);
    setMonthOfYear(tm2Month(t));
    setYear(t.tm_year);
    setHour(t.tm_hour);
    setMinute(t.tm_min);
    setSecond(t.tm_sec);
    setDayOfWeek(tm2DayOfWeek(t));
    setSaveLight(tm2SaveLight(t));

}

 

rtc_periodic_irq_select_t Periodic2RtcPeriodic(Period p) {
    switch(p) {
        case Period::ONCE_EVERY_2_SEC: return RTC_PERIODIC_IRQ_SELECT_2_SECOND;
        case Period::ONCE_EVERY_1_SEC: return RTC_PERIODIC_IRQ_SELECT_1_SECOND;
        case Period::N2_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_2_SECOND;
        case Period::N4_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_4_SECOND;
        case Period::N8_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_8_SECOND;
        case Period::N16_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_16_SECOND;
        case Period::N32_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_32_SECOND;
        case Period::N64_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_64_SECOND;
        case Period::N128_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_128_SECOND;
        case Period::N256_TIMES_EVERY_SEC: return RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_256_SECOND;
        default: return RTC_PERIODIC_IRQ_SELECT_1_SECOND; 
    }
    return RTC_PERIODIC_IRQ_SELECT_1_SECOND;
}




bool RTCTime::setDayOfMonth(int _d) {
    bool rv = false;
    if(_d >= MIN_DAY_OF_MONTH && _d <= MAX_DAY_OF_MONTH) {
        day = _d;
        rv = true;
        stime.tm_mday = day;
        stime.tm_yday = day + yday(year, Month2tm(month));
    }
    return rv;
}

bool RTCTime::setMonthOfYear(Month _m) {
    month = _m;
    stime.tm_mon = Month2tm(month);
    stime.tm_yday = day + yday(year, Month2tm(month));
    return true;
}

bool RTCTime::setYear(int _y) {
    year = _y;
    stime.tm_year = year - TM_YEAR_OFFSET;
    stime.tm_yday = day + yday(year, Month2tm(month));
    return true;
}
bool RTCTime::setHour(int _h) {
    bool rv = false;
    if(_h >= MIN_HOURS && _h <= MAX_HOURS) {
        hours = _h;
        rv = true;
        stime.tm_hour = hours;
    }
    return rv;
}

bool RTCTime::setMinute(int _m) {
    bool rv = false;
    if(_m >= MIN_MINUTES && _m <= MAX_MINUTES) {
        minutes = _m;
        rv = true;
        stime.tm_min = minutes;
    }
    return rv;
}

bool RTCTime::setSecond(int _s) {
    bool rv = false;
    if(_s >= MIN_SECONDS && _s <= MAX_SECONDS) {
        seconds = _s;
        rv = true;
        stime.tm_sec = seconds;
    }
    return rv;
}
bool RTCTime::setDayOfWeek(DayOfWeek d) {
    day_of_week = d;
    stime.tm_wday = DayOfWeek2tm(day_of_week);
    return true;
}

bool RTCTime::setSaveLight(SaveLight sl) {
    save_light = sl;
    stime.tm_isdst = SaveLight2tm(save_light);
    return true;
}

/* getters */
int RTCTime::getDayOfMonth()      { return day; }
Month RTCTime::getMonth()          { return month; }
int RTCTime::getYear()            { return year; }
int RTCTime::getHour()            { return hours; }
int RTCTime::getMinutes()         { return minutes; }
int RTCTime::getSeconds()         { return seconds; }
DayOfWeek RTCTime::getDayOfWeek() { return day_of_week; }

time_t RTCTime::getUnixTime()  { return mktime ( (struct tm *)&stime );}
struct tm RTCTime::getTmTime() { return (struct tm)stime; }

/* -------------------------------------------------------------------------- */
/*                             RTClass                                        */
/* -------------------------------------------------------------------------- */

RTClock::RTClock() : is_initialized{false} { }
RTClock::~RTClock() { }

bool RTClock::begin() {
    if(openRtc()) {
        is_initialized = true;
    }
    else {
        is_initialized = false;
    }
}

bool RTClock::getTime(RTCTime &t) {
    struct tm present;
    if(is_initialized) {
        if( getRtcTime(present) ) {
            t.setTM(present);
            return true;
        }
    }
    return false;
}

bool RTClock::setPeriodicCallback(rtc_cbk_t fnc, Period p) {
    if(is_initialized) {
        onRtcInterrupt();
        setRtcPeriodicClbk(fnc);
        return setRtcPeriodicInterrupt(Periodic2RtcPeriodic(p)); 
    }
    return false;
}

bool RTClock::setAlarmCallback(rtc_cbk_t fnc, RTCTime &t, AlarmMatch &m) {
    if(is_initialized) {
        onRtcInterrupt();
        setRtcAlarmClbk(fnc);
        rtc_alarm_time_t at;
        at.min_match = false;               
        at.sec_match = false;                
        at.hour_match = false;               
        at.mday_match = false;              
        at.mon_match = false;                
        at.year_match = false;               
        at.dayofweek_match = false; 

        struct tm alrm = t.getTmTime();
        
        memcpy(&at.time, &alrm, sizeof(struct tm));
        if(m.isMatchingSecond()) {
            at.sec_match = true;
        }
        else if(m.isMatchingMinute()) {
            at.min_match = true;
        }
        else if(m.isMatchingHour() ) {
            at.hour_match = true;
        }
        else if(m.isMatchingDay() ) {
            at.mday_match = true;
        }
        else if(m.isMatchingMonth()) {
            at.mon_match = true;
        }
        else if(m.isMatchingYear()) {
            at.year_match = true;
        }
        else if(m.isMatchingDayOfWeek()) {
            at.dayofweek_match = true;
        }
        
        return setRtcAlarm(at);
    }
    return false;
}

bool RTClock::isRunning() {
    return isRtcRunning();
}

bool RTClock::setTime(RTCTime &t) {
    if(is_initialized) {
        if(setRtcTime((rtc_time_t)t.getTmTime())) {
            return true;
        }
    }
    return false;
}

bool RTClock::setTimeIfNotRunning(RTCTime &t) {
    if(!isRunning()) {
        return setTime(t);
    }
    return false;
}

#if RTC_HOWMANY > 0
//RTClock RTC;
#endif

