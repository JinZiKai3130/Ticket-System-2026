#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include "time.hpp"
#include <cstring>
#include <fstream>

constexpr int MAXTICKET = 1E6 + 5;

struct Ticket {
  char ticket_id[11]{};
  char train_id[31];
  int status; // 0 success 1 pending 2 refunded
  char start_station[31];
  char end_station[31];
  int departure_time;
  int arrival_time;
  int time_index;
  int num; // 买了几张
  int price;
  char username[21];

  bool operator<(const Ticket &other) {
    return strcmp(ticket_id, other.ticket_id) < 0;
  }

  bool operator==(const Ticket &other) {
    return strcmp(ticket_id, other.ticket_id) == 0;
  }
};

struct trainid_time_to_id {
  char trainid_time[41]{};
  char id[11]{};
  bool operator<(const trainid_time_to_id &other) {
    if (strcmp(trainid_time, other.trainid_time) == 0) {
      return strcmp(id, other.id) < 0;
    }
    return strcmp(trainid_time, other.trainid_time) < 0;
  }

  bool operator==(const trainid_time_to_id &other) {
    return strcmp(trainid_time, other.trainid_time) == 0 &&
           strcmp(id, other.id) == 0;
  }
};

struct UserToId {
  char username[21]{};
  char ticket_id[11]{};

  bool operator<(const UserToId &other) {
    if (strcmp(username, other.username) == 0) {
      // 这里按照从后到前的顺序排列，方便直接使用offset寻找
      return strcmp(ticket_id, other.ticket_id) > 0;
    }
    return strcmp(username, other.username) < 0;
  }

  bool operator==(const UserToId &other) {
    return strcmp(username, other.username) == 0 &&
           strcmp(username, other.username) == 0;
  }
};

class TicketManager {
  BPT<Ticket> id_ticket;
  BPT<trainid_time_to_id> trainid_time_id; // pending list
  BPT<UserToId> user_id;
  int total_ticket_num;
  std::fstream numfilestream;
  std::string ticket_num_file_name;

  void print_ticket(const Ticket &);

  void update_total_ticket_id();

public:
  TicketManager(const std::string &, const std::string &, const std::string &,
                const std::string &);
  // 一共四个文件，其中最后一个用来存储total_ticket_num

  ~TicketManager();

  sjtu::map<std::string, std::string> ticket_parser(const std::string &);

  void buy_ticket(const sjtu::map<std::string, std::string> &, const int &,
                  const int &, const int &, const int &);

  void refund_ticket(sjtu::map<std::string, std::string> &, bool &, bool &,
                     std::string &, int &, int &, std::string &, std::string &);

  void query_order(sjtu::map<std::string, std::string> &);

  void pend_ticket(sjtu::map<std::string, std::string> &, const int &,
                   const int &, const int &, const int &);

  void clean(const std::string &, const std::string &, const std::string &,
             const std::string &);
};