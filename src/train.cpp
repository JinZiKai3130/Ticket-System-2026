#include "../include/train.hpp"
#include "../include/priority_queue.hpp"
#include <cstring>
#include <sstream>
#include <string>

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

// 这里的number参数表示第几天的train
void TrainManager::print_train(const Train &cur_train, const int &number) {
  std::cout << cur_train.id << " " << cur_train.type << '\n';
  for (int i = 0; i < cur_train.station_number; i++) {
    bool flag = (cur_train.station_number - 1 != i);
    std::cout << cur_train.station_name[i] << " "
              << get_abs_time(cur_train.arrival[number][i]) << " -> "
              << get_abs_time(cur_train.departure[number][i]) << " "
              << cur_train.price[i] << " " // price本身就是前缀和数组
              << (flag ? std::to_string(cur_train.left_ticket[number][i + 1])
                       : "x")
              << "\n"; // 这里没有release时应该自动是满票的状态
  }
}

void TrainManager::print_ticket(const Train &cur_train, const int &date_offset,
                                const std::string &start_station,
                                const std::string &end_station,
                                const int &start_index, const int &end_index) {
  std::cout << cur_train.id << " " << start_station << " "
            << get_abs_time(cur_train.departure[date_offset][start_index])
            << " -> " << end_station << " "
            << get_abs_time(cur_train.arrival[date_offset][end_index]) << " "
            << cur_train.price[end_index] - cur_train.price[start_index] << " ";
  int ticket_num = MAXSEAT;
  for (int i = start_index + 1; i <= end_index; i++) {
    ticket_num = std::min(ticket_num, cur_train.left_ticket[date_offset][i]);
  }
  std::cout << ticket_num << "\n";
}

TrainManager::TrainManager(const std::string &file_name_1,
                           const std::string &file_name_2,
                           const std::string &file_name_3,
                           const std::string &file_name_4) {
  id_train.init(file_name_1);
  std::fstream test;
  test.open(file_name_4, std::ios::binary | std::ios::in);
  bool exists = test.good();
  test.close();
  if (!exists) {
    train_data.initialise(file_name_4);
  } else {
    train_data.open_existing(file_name_4);
  }
  route_id.init(file_name_2);
  station_id.init(file_name_3);
}

int TrainManager::add_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<TrainRef>::index_value new_train_ref;
  BPT<route_to_id>::index_value new_route;
  BPT<station_to_id>::index_value new_station;

  std::strcpy(new_route.value.id, info["-i"].c_str());
  std::strcpy(new_station.value.id, info["-i"].c_str());
  std::strcpy(new_train_ref.index, info["-i"].c_str());

  // 如果有这个id，则返回-1
  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, new_train_ref, vec_ref, num);
  if (num != 0) { // 如果id已经存在
    // if (info["-i"] == "AndIwillmakeaso") {
    //   std::cerr << "idAndIwillmakeaso found here\n";
    // }
    return -1;
  }

  Train *new_train_ptr = new Train();
  Train &new_train_data = *new_train_ptr;
  std::strcpy(new_train_data.id, info["-i"].c_str());

  new_train_data.station_number = std::stoi(info["-n"]);
  new_train_data.totoal_seat = std::stoi(info["-m"]);

  // 用数组存储station_name
  std::string parsed_station_name[MAXSTATION];
  parse_string(info["-s"], parsed_station_name);
  for (int i = 0; i < new_train_data.station_number; i++) { // 起点枚举
    std::strcpy(new_train_data.station_name[i], parsed_station_name[i].c_str());
    std::strcpy(new_station.index, parsed_station_name[i].c_str());
    std::strcpy(new_station.value.station, parsed_station_name[i].c_str());
    for (int j = i + 1; j < new_train_data.station_number; j++) { // 终点枚举
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
  new_train_data.price[0] = 0;
  for (int i = 0; i < new_train_data.station_number - 1; i++) {
    new_train_data.price[i + 1] =
        new_train_data.price[i] + std::stoi(parsed_price[i]);
  }

  std::string start_day = info["-d"].substr(0, 5);
  std::string end_day = info["-d"].substr(6, 5);
  int &start_day_num = new_train_data.sale_start = date_to_day_index(start_day);
  int &end_day_num = new_train_data.sale_end = date_to_day_index(end_day);
  new_train_data.departure[0][0] =
      time_to_int(info["-x"]) + start_day_num * DAY_MINUTE;

  std::string travel_time[MAXSTATION]{};
  parse_string(info["-t"], travel_time);
  std::string stop_time[MAXSTATION]{};
  parse_string(info["-o"], stop_time);

  // 这里的i对应parser出来的下标
  for (int day_num = 0; day_num <= end_day_num - start_day_num; day_num++) {
    // 每日的初始化
    new_train_data.departure[day_num][0] =
        new_train_data.departure[0][0] + day_num * DAY_MINUTE;

    for (int i = 0; i < new_train_data.station_number - 1; i++) {
      new_train_data.arrival[day_num][i + 1] =
          new_train_data.departure[day_num][i] + std::stoi(travel_time[i]);

      if (i != new_train_data.station_number - 2)
        new_train_data.departure[day_num][i + 1] =
            new_train_data.arrival[day_num][i + 1] + std::stoi(stop_time[i]);
    }
  }

  for (int i = 0; i <= end_day_num - start_day_num; i++) {
    for (int j = 1; j < new_train_data.station_number; j++) {
      new_train_data.left_ticket[i][j] = new_train_data.totoal_seat;
    }
  }

  new_train_data.is_released = false;
  new_train_data.type = info["-y"][0];

  int offset = train_data.write(new_train_data);
  new_train_ref.value = TrainRef{offset};
  id_train.insert(new_train_ref);
  delete new_train_ptr;
  return 0;
}

int TrainManager::delete_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<TrainRef>::index_value target_train;
  BPT<route_to_id>::index_value target_route;
  BPT<station_to_id>::index_value target_station;

  std::strcpy(target_route.value.id, info["-i"].c_str());
  std::strcpy(target_station.value.id, info["-i"].c_str());
  std::strcpy(target_train.index, info["-i"].c_str());

  // 先找到这个车
  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1) { // 如果找到的车的数量不为1
    return -1;
  }

  TrainRef &the_ref = vec_ref[1];
  Train the_train;
  train_data.read(the_train, the_ref.offset);

  if (the_train.is_released) { // 已发布
    return -1;
  }

  id_train.remove(BPT<TrainRef>::index_value{info["-i"].c_str(), the_ref});
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
  BPT<TrainRef>::index_value target_train;
  std::strcpy(target_train.index, info["-i"].c_str());

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1) { // 如果找到的数量不为1
    return -1;
  }

  TrainRef &the_ref = vec_ref[1];
  Train the_train;
  train_data.read(the_train, the_ref.offset);

  if (the_train.is_released) { // 已发布
    return -1;
  }
  id_train.remove(BPT<TrainRef>::index_value{info["-i"].c_str(), the_ref});
  the_train.is_released = true;
  train_data.update(the_train, the_ref.offset);
  id_train.insert(BPT<TrainRef>::index_value{info["-i"].c_str(), the_ref});
  return 0;
}

int TrainManager::query_train(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);
  BPT<TrainRef>::index_value target_train;

  std::strcpy(target_train.index, info["-i"].c_str());

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1) { // 如果找到的数量不为1
    return -1;
  }

  Train cur_train;
  train_data.read(cur_train, vec_ref[1].offset);

  std::string &query_date = info["-d"];
  int date_index = date_to_day_index(query_date);
  if (date_index < cur_train.sale_start || date_index > cur_train.sale_end) {
    return -1;
  } // 如果不在销售区间内

  print_train(cur_train, date_index - cur_train.sale_start);
  return 0;
}

void TrainManager::query_ticket(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);

  bool is_by_cost = (info["-p"] == "cost");

  std::string the_route = info["-s"] + info["-t"];
  BPT<route_to_id>::index_value target_route;

  strcpy(target_route.index, the_route.c_str());

  route_to_id vec_route[MAXTRAIN];
  int route_num = 0;
  route_id.findinterval(route_id.root, target_route, vec_route, route_num);
  if (route_num == 0) { // 没有这个车
    std::cout << "0\n";
    return;
  }

  sjtu::priority_queue<AvailableTicket, ComparePriceAsc> queue_cost;
  sjtu::priority_queue<AvailableTicket, CompareTimeAsc> queue_time;
  for (int i = 1; i <= route_num; i++) {
    TrainRef vec_ref[3];
    BPT<TrainRef>::index_value target_train;
    strcpy(target_train.index, vec_route[i].id);
    int train_num = 0;
    id_train.findinterval(id_train.root, target_train, vec_ref, train_num);

    Train cur_train;
    train_data.read(cur_train, vec_ref[1].offset);

    if (!cur_train.is_released) {
      continue;
    }

    int start_station_offset = 0;
    int end_station_offset = 0;

    for (int j = 0; j < cur_train.station_number; j++) {
      if (strcmp(info["-s"].c_str(), cur_train.station_name[j]) == 0) {
        start_station_offset = j;
        continue;
      }
      if (strcmp(info["-t"].c_str(), cur_train.station_name[j]) == 0) {
        end_station_offset = j;
        break;
      }
    }

    // 这里确定是否在sale date区域内
    // 对于售票中的每一趟车，都进行一下查找 i.e.枚举始发站发车日期的下标
    for (int j = 0; j <= cur_train.sale_end - cur_train.sale_start; j++) {
      std::string tmp_string =
          get_abs_time(cur_train.departure[j][start_station_offset]);
      tmp_string.resize(5);

      if (strcmp(tmp_string.c_str(), info["-d"].c_str()) == 0) {
        // 这趟车可以进入priority_queue
        AvailableTicket tmp;
        strcpy(tmp.id, cur_train.id);
        tmp.price = cur_train.price[end_station_offset] -
                    cur_train.price[start_station_offset];
        tmp.time = cur_train.arrival[j][end_station_offset] -
                   cur_train.departure[j][start_station_offset];
        tmp.date_offset = j;
        tmp.start_index = start_station_offset;
        tmp.end_index = end_station_offset;
        if (is_by_cost) {
          queue_cost.push(tmp);
        } else {
          queue_time.push(tmp);
        }
        break; // 一个id的车，最多满足一次（每日一班车）
      }
    }
  }

  std::cout << std::max(queue_cost.size(), queue_time.size()) << "\n";
  while (!queue_cost.empty() || !queue_time.empty()) {
    TrainRef vec_ref[3];
    BPT<TrainRef>::index_value target_train;
    int train_num = 0;
    if (is_by_cost) {
      strcpy(target_train.index, queue_cost.top().id);
      id_train.findinterval(id_train.root, target_train, vec_ref, train_num);

      Train cur_train;
      train_data.read(cur_train, vec_ref[1].offset);

      print_ticket(cur_train, queue_cost.top().date_offset, info["-s"],
                   info["-t"], queue_cost.top().start_index,
                   queue_cost.top().end_index);
      queue_cost.pop();
      // std::cout << "successful pop\n";
    } else {
      strcpy(target_train.index, queue_time.top().id);
      id_train.findinterval(id_train.root, target_train, vec_ref, train_num);

      Train cur_train;
      train_data.read(cur_train, vec_ref[1].offset);

      print_ticket(cur_train, queue_time.top().date_offset, info["-s"],
                   info["-t"], queue_time.top().start_index,
                   queue_time.top().end_index);
      queue_time.pop();
    }
  }
}

int TrainManager::query_transfer(const std::string &str) {
  sjtu::map<std::string, std::string> info = train_parser(str);

  bool is_by_cost = (info["-p"] == "cost");

  std::string &start_station_name = info["-s"];
  std::string &end_station_name = info["-t"];
  std::string &departure_date = info["-d"];

  BPT<station_to_id>::index_value target_station;

  strcpy(target_station.index, start_station_name.c_str());

  station_to_id vec_station[MAXTRAIN];
  int station_num = 0;
  station_id.findinterval(station_id.root, target_station, vec_station,
                          station_num);
  if (station_num == 0) { // 没有这个站点
    return -1;
  }

  // step2: 判断经过该站点的车是否合法
  AvailableTicket available_tickets[MAXTRAIN];
  int cnt = 0;
  for (int i = 1; i <= station_num; i++) {
    // 先读出可能的车次，一次进行判断
    TrainRef vec_ref[3];
    BPT<TrainRef>::index_value target_train;
    strcpy(target_train.index, vec_station[i].id);
    int train_num = 0;
    id_train.findinterval(id_train.root, target_train, vec_ref, train_num);

    Train cur_train;
    train_data.read(cur_train, vec_ref[1].offset);
    if (!cur_train.is_released) {
      continue;
    }
    // 合法性1：不是终点
    if (strcmp(cur_train.station_name[cur_train.station_number - 1],
               start_station_name.c_str()) == 0) {
      continue;
    }

    // 合法性2：在sale_date区间内
    int departure_time = date_to_day_index(departure_date);
    int station_offset;
    // 找到对应的station下标
    for (int j = 0; j < cur_train.station_number; j++) {
      if (strcmp(cur_train.station_name[j], start_station_name.c_str()) == 0) {
        station_offset = j;
        break;
      }
    }

    // 判断是否和departure_date相同
    for (int j = 0; j <= cur_train.sale_end - cur_train.sale_start; j++) {
      std::string abs_time =
          get_abs_time(cur_train.departure[j][station_offset]);
      abs_time.resize(5);
      if (abs_time == departure_date) {
        available_tickets[++cnt].date_offset = j;
        available_tickets[cnt].start_index = station_offset;
        strcpy(available_tickets[cnt].id, cur_train.id);
        break;
      }
    }
  }

  if (cnt == 0) {
    return -1;
  }

  TransferTicket ans;
  TransferTicket cur;
  strcpy(ans.ticket1.id, "");
  for (int i = 1; i <= cnt; i++) {
    // 先对于每一个可能的车次拿出来
    TrainRef vec_train1_ref[3];
    BPT<TrainRef>::index_value target_train1;
    strcpy(target_train1.index, available_tickets[i].id);
    int train1_num = 0;
    id_train.findinterval(id_train.root, target_train1, vec_train1_ref,
                          train1_num);

    Train cur_train1;
    train_data.read(cur_train1, vec_train1_ref[1].offset);
    for (int end_station_index = available_tickets[i].start_index + 1;
         end_station_index < cur_train1.station_number; end_station_index++) {
      // 枚举所有train1的可能的终点，即为中转站
      std::string route =
          std::string(cur_train1.station_name[end_station_index]) +
          end_station_name;
      BPT<route_to_id>::index_value target_route;

      strcpy(target_route.index, route.c_str());

      route_to_id vec_route[MAXTRAIN];
      int route_num = 0;
      route_id.findinterval(route_id.root, target_route, vec_route, route_num);

      if (route_num == 0) {
        continue;
      }

      cur.ticket1 = available_tickets[i];
      cur.ticket1.end_index = end_station_index;
      cur.ticket1.price = cur_train1.price[end_station_index] -
                          cur_train1.price[available_tickets[i].start_index];
      cur.ticket1.time =
          cur_train1
              .arrival[available_tickets[i].date_offset][end_station_index] -
          cur_train1.departure[available_tickets[i].date_offset]
                              [available_tickets[i].start_index];

      for (int j = 1; j <= route_num; j++) {
        // 找到第二段路径对应的车
        if (strcmp(vec_route[j].id, cur_train1.id) == 0) {
          continue;
        }
        TrainRef vec_train2_ref[3];
        BPT<TrainRef>::index_value target_train2;

        strcpy(target_train2.index, vec_route[j].id);

        int train2_num = 0;
        id_train.findinterval(id_train.root, target_train2, vec_train2_ref,
                              train2_num);

        Train cur_train2;
        train_data.read(cur_train2, vec_train2_ref[1].offset);

        if (!cur_train2.is_released) {
          continue;
        }

        // 判断这个中转时间是否有这个车，要求时间只能大不能小
        int start2_index = 0, end2_index = 0;
        for (int k = 0; k < cur_train2.station_number; k++) { // 寻找下标
          if (strcmp(cur_train2.station_name[k],
                     cur_train1.station_name[end_station_index]) == 0) {
            start2_index = k;
            continue;
          }
          if (strcmp(cur_train2.station_name[k], info["-t"].c_str()) == 0) {
            end2_index = k;
            break;
          }
        }
        if (start2_index >= end2_index) {
          continue;
        }
        int min_date_index = MAXDAY;
        // 枚举是哪一天第二辆车出发
        for (int k = 0; k <= cur_train2.sale_end - cur_train2.sale_start; k++) {
          if (cur_train2.departure[k][start2_index] >=
              cur_train1.arrival[available_tickets[i].date_offset]
                                [end_station_index]) {
            min_date_index = k;
            break;
          }
        }

        if (min_date_index == MAXDAY) { // train2都在train1到达前离开
          continue;
        }

        cur.ticket2.date_offset = min_date_index;
        cur.ticket2.end_index = end2_index;
        cur.ticket2.start_index = start2_index;
        strcpy(cur.ticket2.id, cur_train2.id);
        cur.ticket2.price =
            cur_train2.price[end2_index] - cur_train2.price[start2_index];
        cur.ticket2.time = cur_train2.arrival[min_date_index][end2_index] -
                           cur_train2.departure[min_date_index][start2_index];

        cur.total_time = cur_train2.arrival[min_date_index][end2_index] -
                         cur_train1.departure[available_tickets[i].date_offset]
                                             [available_tickets[i].start_index];

        if (strcmp(ans.ticket1.id, "") == 0) {
          ans = cur;
        } else if (is_by_cost) {
          if (cur.ticket1.price + cur.ticket2.price <
              ans.ticket1.price + ans.ticket2.price) {
            ans = cur;
          } else if (cur.ticket1.price + cur.ticket2.price ==
                     ans.ticket1.price + ans.ticket2.price) {
            if (cur.total_time < ans.total_time) {
              ans = cur;
            } else if (cur.total_time == ans.total_time) {
              if (strcmp(cur.ticket1.id, ans.ticket1.id) < 0) {
                ans = cur;
              } else if (strcmp(cur.ticket1.id, ans.ticket1.id) == 0) {
                if (strcmp(cur.ticket2.id, ans.ticket2.id) < 0) {
                  ans = cur;
                }
              }
            }
          }
        } else {
          if (cur.total_time < ans.total_time) {
            ans = cur;
          } else if (cur.total_time == ans.total_time) {
            if (cur.ticket1.price + cur.ticket2.price <
                ans.ticket1.price + ans.ticket2.price) {
              ans = cur;
            } else if (cur.ticket1.price + cur.ticket2.price ==
                       ans.ticket1.price + ans.ticket2.price) {
              if (strcmp(cur.ticket1.id, ans.ticket1.id) < 0) {
                ans = cur;
              } else if (strcmp(cur.ticket1.id, ans.ticket1.id) == 0) {
                if (strcmp(cur.ticket2.id, ans.ticket2.id) < 0) {
                  ans = cur;
                }
              }
            }
          }
        }
      }
    }
  }

  if (strcmp(ans.ticket1.id, "") == 0) {
    return -1;
  }
  TrainRef final1_ref[3];
  int final_num1 = 0;
  BPT<TrainRef>::index_value final_train1;
  strcpy(final_train1.index, ans.ticket1.id);
  id_train.findinterval(id_train.root, final_train1, final1_ref, final_num1);

  TrainRef final2_ref[3];
  int final_num2 = 0;
  BPT<TrainRef>::index_value final_train2;
  strcpy(final_train2.index, ans.ticket2.id);
  id_train.findinterval(id_train.root, final_train2, final2_ref, final_num2);

  Train final1;
  train_data.read(final1, final1_ref[1].offset);
  Train final2;
  train_data.read(final2, final2_ref[1].offset);

  print_ticket(final1, ans.ticket1.date_offset,
               final1.station_name[ans.ticket1.start_index],
               final1.station_name[ans.ticket1.end_index],
               ans.ticket1.start_index, ans.ticket1.end_index);
  print_ticket(final2, ans.ticket2.date_offset,
               final2.station_name[ans.ticket2.start_index],
               final2.station_name[ans.ticket2.end_index],
               ans.ticket2.start_index, ans.ticket2.end_index);

  return 0;
}

int TrainManager::get_departure_index(const Train &cur_train,
                                      const std::string &departure_station,
                                      const std::string &departure_date,
                                      int &departure_time) {
  int station_index = 0;
  for (int i = 0; i < cur_train.station_number; i++) {
    if (strcmp(cur_train.station_name[i], departure_station.c_str()) == 0) {
      station_index = i;
      break;
    }
  }

  for (int i = 0; i <= cur_train.sale_end - cur_train.sale_start; i++) {
    std::string abs_time = get_abs_time(cur_train.departure[i][station_index]);
    abs_time.resize(5);
    if (departure_date == abs_time) {
      departure_time = cur_train.departure[i][station_index];
      return i;
    }
  }
  return -1;
}

int TrainManager::check_ticket_enough(sjtu::map<std::string, std::string> &info,
                                      int &departure_time, int &arrival_time,
                                      int &price, int &departure_time_index,
                                      int &start_station_index,
                                      int &end_station_index) {
  // 首先从start_time找到departure_index
  TrainRef vec_ref[3];
  int vec_num = 0;
  BPT<TrainRef>::index_value target_train;
  strcpy(target_train.index, info["-i"].c_str());
  id_train.findinterval(id_train.root, target_train, vec_ref, vec_num);

  if (vec_num < 1)
    return -1;

  Train cur_train;
  train_data.read(cur_train, vec_ref[1].offset);

  if (!cur_train.is_released)
    return -1;

  std::string &departure_station = info["-f"];
  std::string &departure_date = info["-d"];
  departure_time_index = get_departure_index(cur_train, departure_station,
                                             departure_date, departure_time);
  start_station_index = -1;
  end_station_index = -1;

  // std::cout << "departure_time_index = " << departure_time_index << "\n";
  for (int i = 0; i < cur_train.station_number; i++) {
    if (strcmp(cur_train.station_name[i], departure_station.c_str()) == 0) {
      start_station_index = i;
      continue;
    }
    if (strcmp(cur_train.station_name[i], info["-t"].c_str()) == 0) {
      end_station_index = i;
      continue;
    }
  }
  if (start_station_index == -1 || end_station_index == -1 ||
      start_station_index >= end_station_index)
    return -1;

  if (departure_time_index < 0)
    return -1;
  arrival_time = cur_train.arrival[departure_time_index][end_station_index];
  // 再从departure_index确定这一段station是否有充足的票
  int requirement_num = std::stoi(info["-n"]);
  if (requirement_num > cur_train.totoal_seat) {
    return -1;
  }
  price =
      cur_train.price[end_station_index] - cur_train.price[start_station_index];
  for (int i = start_station_index + 1; i <= end_station_index; i++) {
    if (cur_train.left_ticket[departure_time_index][i] < requirement_num) {
      return 0;
    }
  }
  return 1;
}

// 下面两个函数仅负责处理数据，不进行任何安全检查

// num 表示购票/退票数量
// 关于是否有足够的余票应该交给在别处处理
void TrainManager::successful_ticket_purchase(const char *id,
                                              const int &ticket_num,
                                              const int &departure_day,
                                              const int &start_station,
                                              const int &end_station) {
  BPT<TrainRef>::index_value target_train;

  std::strcpy(target_train.index, id);

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1) { // 如果找到的数量不为1
    return;
  }

  TrainRef &cur_ref = vec_ref[1];
  Train cur_train;
  train_data.read(cur_train, cur_ref.offset);
  // if (std::string(cur_train.id) == "AndIwillmakeaso") {
  //   std::cerr << "total seat = " << cur_train.totoal_seat << "\n";
  // }
  id_train.remove(BPT<TrainRef>::index_value{id, cur_ref});

  for (int i = start_station + 1; i <= end_station; i++) {
    cur_train.left_ticket[departure_day][i] -= ticket_num;
  }

  train_data.update(cur_train, cur_ref.offset);
  id_train.insert(BPT<TrainRef>::index_value{id, cur_ref});
}

// 此处refund结束后要处理候补的可能
// 这个函数仅用于删除，候补的购票逻辑和successful_purchase应当一致
void TrainManager::refund_ticket(const char *id, const int &ticket_num,
                                 const int &departure_day,
                                 const std::string &start_station,
                                 const std::string &end_station) {
  BPT<TrainRef>::index_value target_train;

  std::strcpy(target_train.index, id);

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1) { // 如果找到的数量不为1
    return;
  }

  TrainRef &cur_ref = vec_ref[1];
  Train cur_train;
  train_data.read(cur_train, cur_ref.offset);
  id_train.remove(BPT<TrainRef>::index_value{id, cur_ref});

  int start_station_index = 0;
  int end_station_index = 0;
  for (int i = 0; i < cur_train.station_number; i++) {
    if (strcmp(cur_train.station_name[i], start_station.c_str()) == 0) {
      start_station_index = i;
      continue;
    }
    if (strcmp(cur_train.station_name[i], end_station.c_str()) == 0) {
      end_station_index = i;
      break;
    }
  }
  for (int i = start_station_index + 1; i <= end_station_index; i++) {
    cur_train.left_ticket[departure_day][i] += ticket_num;
  }

  train_data.update(cur_train, cur_ref.offset);
  // std::cerr << "WROTE_REFUND " << id << " offset=" << cur_ref.offset
  //           << " day0_seg11=" << cur_train.left_ticket[0][11] << "\n";
  id_train.insert(BPT<TrainRef>::index_value{id, cur_ref});
}

void TrainManager::write_train_back(const char *id,
                                    const Train &updated_train) {
  BPT<TrainRef>::index_value target_train;
  std::strcpy(target_train.index, id);

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num != 1)
    return;

  TrainRef &cur_ref = vec_ref[1];
  id_train.remove(BPT<TrainRef>::index_value{id, cur_ref});
  train_data.update(const_cast<Train &>(updated_train), cur_ref.offset);
  id_train.insert(BPT<TrainRef>::index_value{id, cur_ref});
}

bool TrainManager::get_train_by_id(const char *id, Train &out) {
  BPT<TrainRef>::index_value target_train;
  std::strcpy(target_train.index, id);

  TrainRef vec_ref[3];
  int num = 0;
  id_train.findinterval(id_train.root, target_train, vec_ref, num);
  if (num < 1)
    return false;

  train_data.read(out, vec_ref[1].offset);
  return true;
}

void TrainManager::clean(const std::string &str1, const std::string &str2,
                         const std::string &str3, const std::string &str4) {
  id_train.clean(str1);
  train_data.initialise(str4);
  route_id.clean(str2);
  station_id.clean(str3);
}