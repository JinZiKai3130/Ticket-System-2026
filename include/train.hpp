#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include "time.hpp"
#include <cstring>
#include <string>

constexpr int MAXSTATION = 105;
constexpr int MAXTIME = 144000;
constexpr int MAXDAY = 100; // 30 + 31 + 31 + 7 = 99
constexpr int MAXTRAIN = 1e4 + 5;
constexpr int MAXSEAT = 1e5 + 5;

struct Train {
  // 下标说明：车站为0-(MAXSTATION - 1)
  //          所有区间的性质均跟着后方的点
  char id[21];
  int station_number;
  char station_name[MAXSTATION][35]; // 0 - (MAXSTATION - 1)
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

  bool operator<(const Train &other) const { return strcmp(id, other.id) < 0; }
  bool operator==(const Train &other) const {
    return strcmp(id, other.id) == 0;
  }
};

struct TrainRef {
  int offset;

  bool operator<(const TrainRef &other) const { return offset < other.offset; }
  bool operator==(const TrainRef &other) const {
    return offset == other.offset;
  }
};

struct route_to_id {
  char route[65];
  char id[21];

  bool operator<(const route_to_id &other) const {
    if (strcmp(route, other.route) == 0) {
      return strcmp(id, other.id) < 0;
    }
    return strcmp(route, other.route) < 0;
  }

  bool operator==(const route_to_id &other) const {
    return strcmp(route, other.route) == 0 && strcmp(id, other.id) == 0;
  }

  route_to_id() = default;

  route_to_id(const std::string &r, const std::string &train_id) {
    strcpy(route, r.c_str());
    strcpy(id, train_id.c_str());
  }
};

struct station_to_id {
  char station[35];
  char id[21];

  bool operator<(const station_to_id &other) const {
    if (strcmp(station, other.station) == 0) {
      return strcmp(id, other.id) < 0;
    }
    return strcmp(station, other.station) < 0;
  }

  bool operator==(const station_to_id &other) const {
    return strcmp(station, other.station) == 0 && strcmp(id, other.id) == 0;
  }

  station_to_id() = default;

  station_to_id(const std::string &st, const std::string &train_id) {
    strcpy(station, st.c_str());
    strcpy(id, train_id.c_str());
  }
};

struct AvailableTicket {
  int price;
  int time;
  int date_offset;
  int start_index, end_index;
  char id[21];
};

struct TransferTicket {
  AvailableTicket ticket1;
  AvailableTicket ticket2;
};

struct ComparePriceAsc {
  bool operator()(const AvailableTicket &a, const AvailableTicket &b) const {
    if (a.price == b.price) {
      return std::strcmp(a.id, b.id) > 0;
    }
    return a.price > b.price;
  }
};

struct CompareTimeAsc {
  bool operator()(const AvailableTicket &a, const AvailableTicket &b) const {
    if (a.time == b.time) {
      return std::strcmp(a.id, b.id) > 0;
    }
    return a.time > b.time;
  }
};

class TrainManager {
  BPT<TrainRef> id_train;
  MemoryRiver<Train> train_data;
  BPT<route_to_id> route_id;
  BPT<station_to_id> station_id;
  sjtu::map<std::string, std::string> train_parser(const std::string &);

  void parse_string(const std::string &, std::string (&)[MAXSTATION]);

  void print_train(const Train &, const int &);

  void print_ticket(const Train &, const int &, const std::string &,
                    const std::string &, const int &, const int &);

  int get_departure_index(const Train &, const std::string &,
                          const std::string &, int &);

public:
  TrainManager(const std::string &file_name_1, const std::string &file_name_2,
               const std::string &file_name_3, const std::string &file_name_4);

  int add_train(const std::string &);

  int delete_train(const std::string &);

  int release_train(const std::string &);

  int query_train(const std::string &);

  void query_ticket(const std::string &);

  // num 表示购票/退票数量
  void successful_ticket_purchase(const char *, const int &, const int &,
                                  const int &, const int &);

  int query_transfer(const std::string &);

  // 此处refund结束后要处理候补的可能
  void refund_ticket(const char *, const int &, const int &,
                     const std::string &, const std::string &);

  int check_ticket_enough(sjtu::map<std::string, std::string> &, int &, int &,
                          int &, int &, int &, int &);

  void write_train_back(const char *id, const Train &updated_train);

  bool get_train_by_id(const char *id, Train &out);

  void clean(const std::string &, const std::string &, const std::string &,
             const std::string &);
};