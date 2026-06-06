#include "../include/ticket.hpp"
#include "../include/train.hpp"
#include <sstream>
#include <string>

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
            << cur_ticket.end_station << " "
            << get_abs_time(cur_ticket.arrival_time) << " " << cur_ticket.price
            << " " << cur_ticket.num << '\n';
}

void TicketManager::update_total_ticket_id() {
  numfilestream.seekp(0, std::ios::beg);
  numfilestream.write(reinterpret_cast<char *>(&total_ticket_num),
                      sizeof(total_ticket_num));
  numfilestream.flush();
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
    numfilestream.seekp(0, std::ios::beg);
    numfilestream.write(reinterpret_cast<const char *>(&initValue),
                        sizeof(initValue));
  }
  int value = 0;
  numfilestream.seekg(0, std::ios::beg);
  numfilestream.read(reinterpret_cast<char *>(&value), sizeof(value));
  total_ticket_num = value;
  // std::cerr << "read total_ticket_num = " << total_ticket_num
  //           << "\n";
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
                               const int &arrival_time, const int &price,
                               const int &time_index) {
  Ticket cur_ticket;
  strcpy(cur_ticket.username, info["-u"].c_str());
  strcpy(cur_ticket.train_id, info["-i"].c_str());
  cur_ticket.departure_time = departure_time;
  cur_ticket.arrival_time = arrival_time;
  cur_ticket.time_index = time_index;
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

void TicketManager::refund_ticket(sjtu::map<std::string, std::string> &info,
                                  bool &is_successful, bool &is_pending,
                                  std::string &train_id, int &total_ticket_num,
                                  int &departure_train_index,
                                  std::string &departure_station,
                                  std::string &arrival_station) {
  const std::string &cur_username = info["-u"];
  int offset = 1;
  if (info.find("-n") != info.end()) {
    offset = std::stoi(info["-n"]);
  }

  // 找到ticketid
  BPT<UserToId>::index_value cur_user_to_id;
  strcpy(cur_user_to_id.index, cur_username.c_str());

  UserToId *user_to_id_vec = new UserToId[MAXTICKET];
  int user_to_id_num = 0;
  user_id.findinterval(user_id.root, cur_user_to_id, user_to_id_vec,
                       user_to_id_num);
  if (user_to_id_num < offset) {
    is_successful = 0;
    delete[] user_to_id_vec;
    return;
  }

  // 从ticket_id重新标记对应的票据，标记为refund
  std::string target_ticketid = user_to_id_vec[offset].ticket_id;
  BPT<Ticket>::index_value target_ticket;
  strcpy(target_ticket.index, user_to_id_vec[offset].ticket_id);
  delete[] user_to_id_vec;
  Ticket ticket_vec[3];
  int ticket_num = 0;
  id_ticket.findinterval(id_ticket.root, target_ticket, ticket_vec, ticket_num);

  Ticket &cur_ticket = ticket_vec[1];
  train_id = cur_ticket.train_id;
  total_ticket_num = cur_ticket.num;
  departure_train_index = cur_ticket.time_index;
  departure_station = cur_ticket.start_station;
  arrival_station = cur_ticket.end_station;

  if (cur_ticket.status == 2) {
    is_successful = 0;
    return;
  } else if (cur_ticket.status == 1) {
    is_pending = 1;
    // 在pending_list中删掉对应的订单
    std::string pending_index =
        (cur_ticket.train_id) + std::to_string(cur_ticket.time_index);

    BPT<trainid_time_to_id>::index_value target_pending;
    strcpy(target_pending.index, pending_index.c_str());

    // trainid_time_to_id target_pending_ticket_vec[3];

    // trainid_time_id.remove(BPT<trainid_time_to_id>::index_value(
    //     pending_index.c_str(), target_pending_ticket_vec[1]));
    trainid_time_to_id *target_pending_ticket_vec =
        new trainid_time_to_id[MAXTICKET];
    int target_pending_ticket_num = 0;
    trainid_time_id.findinterval(trainid_time_id.root, target_pending,
                                 target_pending_ticket_vec,
                                 target_pending_ticket_num);
    for (int i = 1; i <= target_pending_ticket_num; i++) {
      if (strcmp(target_pending_ticket_vec[i].id, cur_ticket.ticket_id) == 0) {
        trainid_time_id.remove(BPT<trainid_time_to_id>::index_value(
            pending_index.c_str(), target_pending_ticket_vec[i]));
        break;
      }
    }
    delete[] target_pending_ticket_vec;
  } else {
    is_pending = 0;
  }
  is_successful = 1;
  id_ticket.remove(
      BPT<Ticket>::index_value(target_ticketid.c_str(), cur_ticket));
  cur_ticket.status = 2;
  id_ticket.insert(
      BPT<Ticket>::index_value(target_ticketid.c_str(), cur_ticket));
}

void TicketManager::query_order(sjtu::map<std::string, std::string> &info) {
  std::string cur_username = info["-u"];
  BPT<UserToId>::index_value cur_user_to_id;
  strcpy(cur_user_to_id.index, cur_username.c_str());

  UserToId *user_to_id_vec = new UserToId[MAXTICKET];
  int user_to_id_num = 0;
  user_id.findinterval(user_id.root, cur_user_to_id, user_to_id_vec,
                       user_to_id_num);
  std::cout << user_to_id_num << "\n";

  // std::cerr << "query_order for [" << cur_username << "]:\n";
  // for (int i = 1; i <= user_to_id_num; i++) {
  //   std::cerr << "  [" << i << "] username=" << user_to_id_vec[i].username
  //             << " ticket_id=" << user_to_id_vec[i].ticket_id << "\n";
  // }

  for (int i = 1; i <= user_to_id_num; i++) {
    std::string target_ticketid = user_to_id_vec[i].ticket_id;
    // std::cout << target_ticketid <
    // < "\n";
    BPT<Ticket>::index_value target_ticket;
    strcpy(target_ticket.index, user_to_id_vec[i].ticket_id);
    Ticket ticket_vec[3];
    int ticket_num = 0;
    id_ticket.findinterval(id_ticket.root, target_ticket, ticket_vec,
                           ticket_num);

    // std::cerr << "    -> id_ticket[" << target_ticketid << "] found "
    //           << ticket_num << " entries, username=" <<
    //           ticket_vec[1].username
    //           << "\n";

    Ticket &cur_ticket = ticket_vec[1];
    print_ticket(cur_ticket);
  }
  delete[] user_to_id_vec;
}

void TicketManager::pend_ticket(sjtu::map<std::string, std::string> &info,
                                const int &departure_time,
                                const int &arrival_time, const int &price,
                                const int &time_index) {
  std::string &cur_username = info["-u"];
  std::string &cur_trainid = info["-i"];
  std::string cur_index = cur_trainid + std::to_string(time_index);
  BPT<trainid_time_to_id>::index_value to_be_inserted;
  strcpy(to_be_inserted.index, cur_index.c_str());
  strcpy(to_be_inserted.value.trainid_time, cur_index.c_str());
  strcpy(to_be_inserted.value.id, std::to_string(++total_ticket_num).c_str());
  update_total_ticket_id();
  trainid_time_id.insert(to_be_inserted);

  Ticket cur_ticket;
  strcpy(cur_ticket.username, info["-u"].c_str());
  strcpy(cur_ticket.train_id, info["-i"].c_str());
  cur_ticket.departure_time = departure_time;
  cur_ticket.arrival_time = arrival_time;
  cur_ticket.time_index = time_index;
  strcpy(cur_ticket.start_station, info["-f"].c_str());
  strcpy(cur_ticket.end_station, info["-t"].c_str());
  cur_ticket.num = std::stoi(info["-n"]);
  cur_ticket.price = price;
  cur_ticket.status = 1;
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

void TicketManager::process_pending_refund(const std::string &train_id,
                                           int time_index, Train &cur_train) {
  std::string cur_index = train_id + std::to_string(time_index);

  BPT<trainid_time_to_id>::index_value target_pending;
  strcpy(target_pending.index, cur_index.c_str());

  trainid_time_to_id *pending_vec = new trainid_time_to_id[MAXTICKET];
  int pending_num = 0;
  trainid_time_id.findinterval(trainid_time_id.root, target_pending,
                               pending_vec, pending_num);

  if (pending_num == 0) {
    delete[] pending_vec;
    return;
  }

  for (int i = 1; i <= pending_num; i++) {
    for (int j = i + 1; j <= pending_num; j++) {
      if (std::stoi(pending_vec[i].id) > std::stoi(pending_vec[j].id)) {
        trainid_time_to_id tmp = pending_vec[i];
        pending_vec[i] = pending_vec[j];
        pending_vec[j] = tmp;
      }
    }
  }

  for (int i = 1; i <= pending_num; i++) {
    // 读取候补订单对应的 Ticket
    BPT<Ticket>::index_value target_ticket;
    strcpy(target_ticket.index, pending_vec[i].id);

    Ticket ticket_vec[3];
    int ticket_found = 0;
    id_ticket.findinterval(id_ticket.root, target_ticket, ticket_vec,
                           ticket_found);

    if (ticket_found < 1)
      continue;

    Ticket &cur_ticket = ticket_vec[1];
    if (cur_ticket.status != 1)
      continue;

    // 找到出发站和到达站在车次中的下标
    int start_idx = -1, end_idx = -1;
    for (int j = 0; j < cur_train.station_number; j++) {
      if (strcmp(cur_train.station_name[j], cur_ticket.start_station) == 0) {
        start_idx = j;
        continue;
      }
      if (strcmp(cur_train.station_name[j], cur_ticket.end_station) == 0) {
        end_idx = j;
        continue;
      }
    }

    if (start_idx == -1 || end_idx == -1 || start_idx >= end_idx)
      continue;

    // 检查区域上的余票
    bool enough = true;
    for (int j = start_idx + 1; j <= end_idx; j++) {
      if (cur_train.left_ticket[time_index][j] < cur_ticket.num) {
        enough = false;
        break;
      }
    }
    if (!enough)
      continue;

    for (int j = start_idx + 1; j <= end_idx; j++) {
      cur_train.left_ticket[time_index][j] -= cur_ticket.num;
    }

    trainid_time_id.remove(BPT<trainid_time_to_id>::index_value(
        cur_index.c_str(), pending_vec[i]));

    id_ticket.remove(BPT<Ticket>::index_value(pending_vec[i].id, cur_ticket));
    cur_ticket.status = 0;
    id_ticket.insert(BPT<Ticket>::index_value(pending_vec[i].id, cur_ticket));
  }
  delete[] pending_vec;
}

void TicketManager::clean(const std::string &str1, const std::string &str2,
                          const std::string &str3, const std::string &str4) {
  id_ticket.clean(str1);
  trainid_time_id.clean(str2);
  user_id.clean(str3);

  int init_value = 0;
  numfilestream.seekp(0, std::ios::beg);
  numfilestream.write(reinterpret_cast<const char *>(&init_value),
                      sizeof(init_value));
  numfilestream.flush();
  total_ticket_num = 0;
}