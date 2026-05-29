#include "../include/train.hpp"
#include <cstring>
#include <sstream>

constexpr int DAY_MINUTE = 24 * 60;

sjtu::map<std::string, std::string>
TrainManager::train_parser(const std::string &str) {
  sjtu::map<std::string, std::string> info;
  std::stringstream ss(str);
  std::string token;
  std::string type;

  while (ss >> type) {
    ss >> token;
    info[type] = token;
  }
  return info;
}

int TrainManager::time_to_int(const std::string &str) {
  std::string hour = "";
  std::string minute = "";
  hour += str[0];
  hour += str[1];
  minute += str[3];
  minute += str[4];
  return std::stoi(hour) * 60 + std::stoi(minute);
}

int TrainManager::date_to_int(const std::string &str) {
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

void TrainManager::parse_string(const std::string &input,
                                std::string (&output)[MAXSTATION]) {
  if (input == "_") {
    output[0] = "";
  }
  int ptr = 0;
  for (int i = 0; i < input.size(); i++) {
    if (input[i] == '|') {
      ptr++;
      continue;
    }
    output[ptr] += input[i];
  }
}

TrainManager::TrainManager(const std::string &file_name_1,
                           const std::string &file_name_2,
                           const std::string &file_name_3) {
  id_train.init(file_name_1);
  route_id.init(file_name_2);
  station_id.init(file_name_3);
}
/*
-i -n -m -s -p -x -t -o -d -y

说明

添加 <trainID> 为 -i，<stationNum> 为 -n，<seatNum> 为 -m，<stations> 为
-s，<prices> 为 -p，<startTime> 为 -x，<travelTimes> 为 -t，<stopoverTimes> 为
-o，<saleDate> 为 -d，<type> 为 -y 的车次。 由于 -s、-p、-t、-o 和 -d
由多个值组成，输入时两个值之间以 | 隔开（仍是一个不含空格的字符串）。

输入保证火车的座位数大于 0,站的数量不少于 2 不多于 100，且如果火车只有两站 -o
后的参数用下划线代替（见举例2）,且火车不会经过同一个站两次。 如果 <trainID>
已经存在则添加失败。
*/
int TrainManager::add_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<Train>::index_value new_train;
  BPT<route_to_id>::index_value new_route;
  BPT<station_to_id>::index_value new_station;

  std::strcpy(new_route.value.id, info["-i"].c_str());
  std::strcpy(new_station.value.id, info["-i"].c_str());
  std::strcpy(new_train.index, info["-i"].c_str());
  std::strcpy(new_train.value.id, info["-i"].c_str());

  // 如果有这个id，则返回-1
  Train vec[3];
  int num = 0;
  id_train.findinterval(id_train.root, new_train, vec, num);
  if (num != 0) { // 如果id已经存在
    return -1;
  }

  new_train.value.station_number = std::stoi(info["-n"]);
  new_train.value.totoal_seat = std::stoi(info["-m"]);

  // 用数组存储station_name
  std::string parsed_station_name[MAXSTATION];
  parse_string(info["-s"], parsed_station_name);
  for (int i = 0; i < new_train.value.station_number; i++) { // 起点枚举
    std::strcpy(new_train.value.station_name[i],
                parsed_station_name[i].c_str());
    std::strcpy(new_station.index, parsed_station_name[i].c_str());
    std::strcpy(new_station.value.station, parsed_station_name[i].c_str());
    for (int j = i + 1; j < new_train.value.station_number; j++) { // 终点枚举
      // 保证起点在前，终点在后
      std::string the_route = parsed_station_name[i] + parsed_station_name[j];
      std::strcpy(new_route.index, the_route.c_str());
      std::strcpy(new_route.value.route, the_route.c_str());
      route_id.insert(new_route);
    }
    station_id.insert(new_station);
  }

  // 用前缀和数组price
  std::string parsed_price[MAXSTATION];
  parse_string(info["-p"], parsed_price);
  new_train.value.price[0] = 0;
  for (int i = 0; i < new_train.value.station_number - 1; i++) {
    new_train.value.price[i + 1] =
        new_train.value.price[i] + std::stoi(parsed_price[i]);
  }

  std::string start_day = info["-d"].substr(0, 5);
  std::string end_day = info["-d"].substr(6, 5);
  int &start_day_num = new_train.value.sale_start = date_to_int(start_day);
  int &end_day_num = new_train.value.sale_end = date_to_int(end_day);
  new_train.value.departure[0][0] =
      time_to_int(info["-x"]) + start_day_num * DAY_MINUTE;

  //   std::string &travel_time = info["-t"];
  //   std::string &stop_time = info["-o"];
  //   int ptr1 = 0, ptr2 = 0; // 在string中的指针
  std::string travel_time[MAXSTATION]{};
  parse_string(info["-t"], travel_time);
  std::string stop_time[MAXSTATION]{};
  parse_string(info["-o"], stop_time);

  // 这里的i对应parser出来的下标
  for (int day_num = 0; day_num <= end_day_num - start_day_num; day_num++) {
    // 每日的初始化
    new_train.value.departure[day_num][0] =
        new_train.value.departure[0][0] + (day_num)*DAY_MINUTE;

    for (int i = 0; i < new_train.value.station_number - 1; i++) {
      new_train.value.arrival[day_num][i + 1] =
          new_train.value.departure[day_num][i] + std::stoi(travel_time[i]);

      if (i != new_train.value.station_number - 2)
        new_train.value.departure[day_num][i + 1] =
            new_train.value.arrival[day_num][i + 1] + std::stoi(stop_time[i]);
    }
  }

  for (int i = 0; i <= end_day_num - start_day_num; i++) {
    for (int j = 1; j < new_train.value.station_number; i++) {
      new_train.value.left_ticket[i][j] = new_train.value.totoal_seat;
    }
  }

  new_train.value.is_released = false;
  new_train.value.type = info["-g"][0];

  id_train.insert(new_train);
  return 0;
}

int TrainManager::delete_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<Train>::index_value target_train;
  BPT<route_to_id>::index_value target_route;
  BPT<station_to_id>::index_value target_station;

  std::strcpy(target_route.value.id, info["-i"].c_str());
  std::strcpy(target_station.value.id, info["-i"].c_str());
  std::strcpy(target_train.index, info["-i"].c_str());
  std::strcpy(target_train.value.id, info["-i"].c_str());

  // 先找到这个车
  Train vec[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec, num);
  if (num != 1) { // 如果找到的车的数量不为1
    return -1;
  }

  Train &the_train = vec[0];
  if (the_train.is_released) { // 已发布
    return -1;
  }

  id_train.remove(BPT<Train>::index_value{info["-i"].c_str(), the_train});
  for (int i = 0; i < the_train.station_number; i++) { // 起点枚举
    strcpy(target_station.index, the_train.station_name[i]);
    strcpy(target_station.value.station, the_train.station_name[i]);
    for (int j = i + 1; j < the_train.station_number; j++) { // 终点枚举
      // 保证起点在前，终点在后
      std::string the_route = std::string(the_train.station_name[i]) +
                              std::string(the_train.station_name[j]);
      strcpy(target_route.index, the_route.c_str());
      strcpy(target_route.value.route, the_route.c_str());
      route_id.remove(target_route);
    }
    station_id.remove(target_station);
  }
  return 0;
}

int TrainManager::release_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<Train>::index_value target_train;
  std::strcpy(target_train.index, info["-i"].c_str());
  std::strcpy(target_train.value.id, info["-i"].c_str());

  Train vec[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec, num);
  if (num != 1) { // 如果找到的数量不为1
    return -1;
  }

  Train &the_train = vec[0];
  if (the_train.is_released) { // 已发布
    return -1;
  }
  id_train.remove(BPT<Train>::index_value{info["-i"].c_str(), the_train});
  the_train.is_released = true;
  id_train.insert(BPT<Train>::index_value{info["-i"].c_str(), the_train});
  return 0;
}

int TrainManager::query_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  return 0;
}

int TrainManager::query_ticket(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  return 0;
}

// num 表示购票/退票数量
void TrainManager::successful_ticket_purchase(const char *id, const int &num,
                                              const int &departure_day,
                                              const int &start_station,
                                              const int &end_station) {}

int TrainManager::query_transfer(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  return 0;
}

// 此处refund结束后要处理候补的可能
void TrainManager::refund(const char *id, const int &num,
                          const int &departure_day, const int &start_station,
                          const int &end_station) {}

void TrainManager::clean(const std::string &str) {
  id_train.clean(str);
  route_id.clean(str);
  station_id.clean(str);
}