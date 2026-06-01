#include "../include/ticket.hpp"
#include <sstream>

void TicketManager::print_ticket(const Ticket &cur_ticket) {
  if (cur_ticket.status == 0) {
    std::cout << "[success] ";
  } else if (cur_ticket.status == 1) {
    std::cout << "[pending] ";
  } else {
    std::cout << "[refunded] ";
  }
  std::cout << cur_ticket.train_id << " " << cur_ticket.start_station << " "
            << get_abs_time(cur_ticket.departure_time) << " -> "
            << cur_ticket.end_station << " " << cur_ticket.price << " "
            << cur_ticket.num << '\n';
}

void TicketManager::update_total_ticket_id() {
  numfilestream.write(reinterpret_cast<char *>(&total_ticket_num),
                      sizeof(total_ticket_num));
}

TicketManager::TicketManager(const std::string &file_name1,
                             const std::string &file_name2,
                             const std::string &file_name3,
                             const std::string &file_name4) {
  id_ticket.init(file_name1);
  trainid_time_id.init(file_name2);
  user_id.init(file_name3);

  numfilestream.open(file_name4.c_str(),
                     std::ios::in | std::ios::out | std::ios::binary);
  if (!numfilestream.is_open()) {
    numfilestream.clear();
    numfilestream.open(file_name4, std::ios::out | std::ios::binary);
    numfilestream.close();
    numfilestream.open(file_name4.c_str(),
                       std::ios::in | std::ios::out | std::ios::binary);
    int initValue = 0;
    numfilestream.write(reinterpret_cast<const char *>(&initValue),
                        sizeof(initValue));
  }
  int value = 0;
  numfilestream.read(reinterpret_cast<char *>(&value), sizeof(value));
  total_ticket_num = value;
  ticket_num_file_name = file_name4;
}

TicketManager::~TicketManager() { numfilestream.close(); }

sjtu::map<std::string, std::string>
TicketManager::ticket_parser(const std::string &input) {
  sjtu::map<std::string, std::string> info;
  std::stringstream ss(input);
  std::string token;
  std::string type;

  while (ss >> type) {
    ss >> token;
    info[type] = token;
  }
  return info;
}

void TicketManager::buy_ticket(const sjtu::map<std::string, std::string> &info,
                               const int &departure_time,
                               const int &arrival_time, const int &price) {
  Ticket cur_ticket;
  strcpy(cur_ticket.username, info["-u"].c_str());
  strcpy(cur_ticket.train_id, info["-i"].c_str());
  cur_ticket.departure_time = departure_time;
  cur_ticket.arrival_time = arrival_time;
  strcpy(cur_ticket.start_station, info["-f"].c_str());
  strcpy(cur_ticket.end_station, info["-t"].c_str());
  cur_ticket.num = std::stoi(info["-n"]);
  cur_ticket.price = price;
  cur_ticket.status = 0;
  total_ticket_num++;
  update_total_ticket_id();
  strcpy(cur_ticket.ticket_id, std::to_string(total_ticket_num).c_str());

  BPT<Ticket>::index_value cur_id_ticket;
  strcpy(cur_id_ticket.index, std::to_string(total_ticket_num).c_str());
  cur_id_ticket.value = cur_ticket;
  id_ticket.insert(cur_id_ticket);

  BPT<UserToId>::index_value cur_user_id;
  strcpy(cur_user_id.index, info["-u"].c_str());
  strcpy(cur_user_id.value.username, info["-u"].c_str());
  strcpy(cur_user_id.value.ticket_id, std::to_string(total_ticket_num).c_str());
  user_id.insert(cur_user_id);
}

void TicketManager::refund_ticket(sjtu::map<std::string, std::string> &info) {
  const std::string &cur_username = info["-u"];
  int offset = 0;
  if (info.find("-n") != info.end()) {
    offset = std::stoi(info["-n"]);
  }
}

void TicketManager::clean(const std::string &str1, const std::string &str2,
                          const std::string &str3, const std::string &str4) {
  id_ticket.clean(str1);
  trainid_time_id.clean(str2);
  user_id.clean(str3);

  int init_value = 0;
  numfilestream.write(reinterpret_cast<const char *>(&init_value),
                      sizeof(init_value));
  total_ticket_num = 0;
}