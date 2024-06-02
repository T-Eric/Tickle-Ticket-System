#ifndef SJTU_CALENDAR_HPP
#define SJTU_CALENDAR_HPP

#include "utils.hpp"

namespace sjtu {

const int MonthDays[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct DateTime;
struct Time;

struct Date {
  int month, date;

  Date()
      : month(6), date(1) {}
  Date(const int& m, const int& d)
      : month(m), date(d) {}
  Date(const string& s) {
    int pos = s.find('-');
    month = stoi(s.substr(0, pos));
    date = stoi(s.substr(pos + 1));
  }
  Date(const Date& d)
      : month(d.month), date(d.date) {}
  ~Date() = default;
  Date& operator=(const Date& d) {
    if (this != &d) {
      month = d.month;
      date = d.date;
    }
    return *this;
  }
  Date& operator=(const string& s) {
    int pos = s.find('-');
    month = stoi(s.substr(0, pos));
    date = stoi(s.substr(pos + 1));
    return *this;
  }
  bool operator==(const Date& d) const {
    return month == d.month && date == d.date;
  }
  bool operator!=(const Date& d) const {
    return month != d.month || date != d.date;
  }
  bool operator<(const Date& d) const {
    return month < d.month || (month == d.month && date < d.date);
  }
  bool operator<=(const Date& d) const {
    return month < d.month || (month == d.month && date <= d.date);
  }
  Date& operator+=(const int& dd) {
    date += dd;
    while (date > MonthDays[month]) {
      date -= MonthDays[month++];
      if (month > 12)
        month = 1;
    }
    return *this;
  }
  Date& operator-=(const int& dd) {
    date -= dd;
    while (date <= 0) {
      date += MonthDays[--month];
      if (month < 1)
        month = 12;
    }
    return *this;
  }

  friend Date operator+(const Date& lhs, const int& dd) {
    Date ret = lhs;
    ret += dd;
    return ret;
  }
  friend Date operator-(const Date& lhs, const int& dd) {
    Date ret = lhs;
    ret -= dd;
    return ret;
  }
  // 不考虑跨年
  friend int operator-(const Date& lhs, const Date& rhs) {
    int ret = 0;
    if (lhs <= rhs) {
      if (rhs.month != lhs.month) {
        int tmp = lhs.month;
        ret += (MonthDays[tmp++] - lhs.date);
        while (tmp < rhs.month)
          ret += MonthDays[tmp++];
        ret += rhs.date;
      } else
        ret += rhs.date - lhs.date;
    } else {
      if (lhs.month != rhs.month) {
        int tmp = rhs.month;
        ret += (MonthDays[tmp++] - rhs.date);
        while (tmp < lhs.month)
          ret += MonthDays[tmp++];
        ret += lhs.date;
      } else
        ret += lhs.date - rhs.date;
      // ret *= -1;
    }
    return ret;
  }
  friend std::ostream& operator<<(std::ostream& os, const Date& d) {
    os << d.month / 10 << d.month % 10 << '-' << d.date / 10 << d.date % 10;
    return os;
  }
};

struct Time {
  int hour, minute;
  int days;  // 暂存经过的天数

  Time()
      : hour(0), minute(0), days(0) {}
  Time(const int& h, const int& m)
      : hour(h), minute(m), days(0) {}
  Time(const string& s) {
    int pos = s.find(':');
    hour = stoi(s.substr(0, pos));
    minute = stoi(s.substr(pos + 1));
    days = 0;
  }
  Time(const Time& t)
      : hour(t.hour), minute(t.minute), days(t.days) {}
  ~Time() = default;

  Time& operator=(const Time& t) {
    if (this != &t) {
      hour = t.hour;
      minute = t.minute;
      days = t.days;
    }
    return *this;
  }
  Time& operator=(const string& s) {
    int pos = s.find(':');
    hour = stoi(s.substr(0, pos));
    minute = stoi(s.substr(pos + 1));
    days = 0;
    return *this;
  }
  bool operator==(const Time& t) const {
    return days == t.days && hour == t.hour && minute == t.minute;
  }
  bool operator!=(const Time& t) const {
    return hour != t.hour || minute != t.minute || days != t.days;
  }
  bool operator<(const Time& t) const {
    return days < t.days || (days == t.days && hour < t.hour) ||
           (days == t.days && hour == t.hour && minute < t.minute);
  }
  bool operator<=(const Time& t) const {
    return days < t.days || (days == t.days && hour < t.hour) ||
           (days == t.days && hour == t.hour && minute <= t.minute);
  }
  Time& operator+=(const int& mm) {
    minute += mm;
    hour += minute / 60;
    minute %= 60;
    days += hour / 24;
    hour %= 24;
    return *this;
  }
  Time& operator-=(const int& mm) {
    minute -= mm;
    hour -= minute / 60;
    minute %= 60;
    days -= hour / 24;
    hour %= 24;
    return *this;
  }
  friend Time operator+(const Time& lhs, const int& mm) {
    Time ret = lhs;
    ret += mm;
    return ret;
  }
  friend Time operator-(const Time& lhs, const int& mm) {
    Time ret = lhs;
    ret -= mm;
    return ret;
  }
  // 考虑跨天，求得是差值
  friend int operator-(const Time& lhs, const Time& rhs) {
    int ret = 0;
    if (lhs < rhs) {
      int tmp = lhs.hour;
      ret += (60 - lhs.minute + rhs.minute);  // 来到下个小时
      ++tmp;
      ret += ((rhs.hour - tmp) * 60 + (rhs.days - lhs.days) * 1440);
    } else {
      int tmp = rhs.hour;
      ret += (60 - rhs.minute + lhs.minute);  // 来到下个小时
      ++tmp;
      ret += ((lhs.hour - tmp) * 60 + (lhs.days - rhs.days) * 1440);
    }
    return ret;
  }
  friend std::ostream& operator<<(std::ostream& os, const Time& t) {
    os << t.hour / 10 << t.hour % 10 << ':' << t.minute / 10 << t.minute % 10;
    return os;
  }
};

struct DateTime {
  Date date;
  Time time;

  DateTime()
      : date(6, 1), time(0, 0) {}
  DateTime(const int& d, const int& m, const int& hh, const int& mm)
      : date(d, m), time(hh, mm) {}
  DateTime(const Date& d, const Time& t)
      : date(d), time(t) {
    date += time.days;
    time.days = 0;
  }
  DateTime(const DateTime& dt)
      : date(dt.date), time(dt.time) {}
  DateTime(const string& d, const string& t)
      : date(d), time(t) {
    date += time.days;
    time.days = 0;
  }
  DateTime(const string& dt) {
    int pos = dt.find(' ');
    date = dt.substr(0, pos);
    time = dt.substr(pos + 1);
    date += time.days;
    time.days = 0;
  }
  DateTime& operator=(const DateTime& dt) {
    if (this != &dt) {
      date = dt.date;
      time = dt.time;
    }
    return *this;
  }
  DateTime& operator=(const string& dt) {
    int pos = dt.find(' ');
    date = dt.substr(0, pos);
    time = dt.substr(pos + 1);
    date += time.days;
    time.days = 0;
    return *this;
  }
  ~DateTime() = default;

  bool operator==(const DateTime& dt) const {
    return date == dt.date && time == dt.time;
  }
  bool operator!=(const DateTime& dt) const {
    return date != dt.date || time != dt.time;
  }
  bool operator<(const DateTime& dt) const {
    return date < dt.date || (date == dt.date && time < dt.time);
  }
  bool operator<=(const DateTime& dt) const {
    return date < dt.date || (date == dt.date && time <= dt.time);
  }
  friend bool operator<(const DateTime& lhs, const Date& rhs) {
    return lhs.date < rhs;
  }
  DateTime& operator+=(const int& t) {
    time += t;
    date += time.days;
    time.days = 0;
    return *this;
  }
  DateTime& operator-=(const int& t) {
    time -= t;
    date += time.days;
    time.days = 0;
    return *this;
  }

  friend DateTime operator+(const DateTime& lhs, const int& t) {
    DateTime ret = lhs;
    ret += t;
    return ret;
  }
  friend DateTime operator-(const DateTime& lhs, const int& t) {
    DateTime ret = lhs;
    ret -= t;
    return ret;
  }
  friend int operator-(const DateTime& lhs, const DateTime& rhs) {
    return (lhs.date - rhs.date) * 1440 + (lhs.time - rhs.time);
  }
  friend std::ostream& operator<<(std::ostream& os, const DateTime& dt) {
    os << dt.date << ' ' << dt.time;
    return os;
  }
};

}  // namespace sjtu

#endif  // !SJTU_CALENDAR_HPP