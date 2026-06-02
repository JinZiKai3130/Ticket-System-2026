#pragma once
#include <cstring>
#include <iostream>

constexpr int DAY_MINUTE = 24 * 60;

inline int time_to_int(const std::string &str) {
  std::string hour = "";
  std::string minute = "";
  hour += str[0];
  hour += str[1];
  minute += str[3];
  minute += str[4];
  return std::stoi(hour) * 60 + std::stoi(minute);
}

inline int date_to_day_index(const std::string &str) {
  std::string month = "";
  std::string day = "";
  month += str[0];
  month += str[1];
  day += str[3];
  day += str[4];
  int result = 0;
  if (month == "06") {
    result += 0;
  } else if (month == "07") {
    result += 30;
  } else {
    result += 61;
  }

  result += std::stoi(day);
  return result;
}

inline std::string to_2_digit_string(const int &cur) {
  if (cur < 10) {
    return "0" + std::to_string(cur);
  } else
    return std::to_string(cur);
}

inline std::string int_to_time(const int &cur_num) {
  int time_in_day = cur_num % DAY_MINUTE;
  int hour = time_in_day / 60;
  int minute = time_in_day % 60;
  return to_2_digit_string(hour) + ":" + to_2_digit_string(minute);
}

inline std::string day_index_to_day(const int &cur_num) {
  int month, day;
  int number = cur_num;
  std::string result = "";
  if (cur_num <= 30) {
    result += "06";
  } else if (cur_num <= 61) {
    result += "07";
    number -= 30;
  } else if (cur_num <= 92) {
    result += "08";
    number -= 61;
  } else {
    result += "09";
    number -= 92;
  }
  result += "-" + to_2_digit_string(number);
  return result;
}

inline std::string get_abs_time(const int &cur_num) {
  std::string result;
  if (cur_num == 0) {
    return "xx-xx xx:xx";
  }
  result += day_index_to_day(cur_num / DAY_MINUTE);
  result += " ";
  result += int_to_time(cur_num);
  return result;
}