#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include <cstring>

struct ticket {
  char ticket_id[11];
  char train_id[31];
  int status; // 0 success 1 pending 2 refunded
  int start_station;
  int end_station;
  int num; // 买了几张
  int price;
  char username[21];

  bool operator<(const ticket &other) {
    return strcmp(ticket_id, other.ticket_id) < 0;
  }
};

struct trainid_time_to_id {
  char trainid_time[41];
  char id[11];
  bool operator<(const trainid_time_to_id &other) {
    return strcmp(trainid_time, other.trainid_time);
  }
};

class TicketManager {
  BPT<ticket> id_ticket;
  BPT<trainid_time_to_id> trainid_time_id; // pending list
  BPT<ticket> user_ticket;

  sjtu::map<std::string, std::string> ticket_parser(const std::string &);

public:
  TicketManager(const std::string &file_name_1, const std::string &file_name_2,
                const std::string &file_name_3);

  void buy_ticket(const std::string &);

  void refund_ticket(const std::string &);

  void clean(const std::string &);
};