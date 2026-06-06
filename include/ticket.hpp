#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include "time.hpp"
#include <cstring>
#include <fstream>

struct Train;
constexpr int MAXTICKET = 1E4 + 5;

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

  bool operator<(const Ticket &other) const {
    return strcmp(ticket_id, other.ticket_id) < 0;
  }

  bool operator==(const Ticket &other) const {
    return strcmp(ticket_id, other.ticket_id) == 0;
  }
};

struct trainid_time_to_id {
  char id[11]{};
  bool operator<(const trainid_time_to_id &other) const {
    int cur = 0, other_cur;
    if (id[0] != '\0')
      cur = std::stoi(id);
    if (other.id[0] != '\0')
      other_cur = std::stoi(other.id);
    return cur > other_cur;
  }

  bool operator==(const trainid_time_to_id &other) const {
    return strcmp(id, other.id) == 0;
  }
};

struct UserToId {
  char ticket_id[11]{};

  bool operator<(const UserToId &other) const {
    int cur = 0, other_cur;
    if (ticket_id[0] != '\0')
      cur = std::stoi(ticket_id);
    if (other.ticket_id[0] != '\0')
      other_cur = std::stoi(other.ticket_id);
    return cur > other_cur;
  }

  bool operator==(const UserToId &other) const {
    return strcmp(ticket_id, other.ticket_id) == 0;
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

  void process_pending_refund(const std::string &train_id, int time_index,
                              Train &cur_train);

  void clean(const std::string &, const std::string &, const std::string &,
             const std::string &);
};