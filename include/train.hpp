#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include <cstring>

constexpr int MAXSTATION = 101;
constexpr int MAXTIME = 144000;
constexpr int MAXDAY = 100; // 30 + 31 + 31 + 7 = 99

struct train {
  char id[21];
  int station_number;
  char station_name[MAXSTATION][31]; // 0 - (MAXSTATION - 1)
  int totoal_seat;
  int left_ticket[MAXDAY][MAXSTATION]; // 1 - (MAXSTATION - 1)
  int price[MAXSTATION]; // 1 - (MAXSTATION - 1) 做成前缀和数组，快速得到区间和
  int sale_start;
  int sale_end;
  bool is_released;
  char type;
  int arrival[MAXDAY][MAXSTATION];   // 1 - (MAXSTATION -1)
  int departure[MAXDAY][MAXSTATION]; // 0 - (MAXSTATION - 2)
  // 第二个量表示出发的站点，具体的下标范围见后方
  // 第一个量表示距离saledate的日期，以确保是哪一天发车的车次

  bool operator<(const train &other) { return strcmp(id, other.id) < 0; }
};

struct route_to_id {
  char route[63];
  char id[21];
};

struct station_to_id {
  char station[31];
  char id[21];
};

class TrainManager {
  BPT<train> id_train;
  BPT<route_to_id> route_id;
  BPT<station_to_id> station_id;
  sjtu::map<std::string, std::string> train_parser(const std::string &);

public:
  TrainManager(const string &file_name_1, const string &file_name_2,
               const string &file_name_3);

  void add_train(const string &);

  void delete_train(const string &);

  void release_train(const string &);

  void query_train(const string &);

  void query_ticket(const string &);

  // num 表示购票/退票数量
  void successful_ticket_purchase(const char *id, const int &num,
                                  const int &departure_day,
                                  const int &start_station,
                                  const int &end_station);

  void query_transfer(const string &);

  // 此处refund结束后要处理候补的可能
  void refund(const char *id, const int &num, const int &departure_day,
              const int &start_station, const int &end_station);

  void clean();
};