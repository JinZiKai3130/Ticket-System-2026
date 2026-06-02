#include "../include/ticket.hpp"
#include "../include/train.hpp"
#include "../include/user.hpp"
#include <iostream>

const std::string user_file_name = "username_user";
const std::string train_id_file_name = "id_Train";
const std::string route_id_file_name = "route_id";
const std::string station_id_file_name = "station_id";
const std::string ticket_id_file_name = "id_Ticket";
const std::string trainid_time_ticketid_file_name = "pending_list";
const std::string user_trainid_file_name = "username_trainid";
const std::string number_of_orders = "order_count";

int main() {
  std::string input;
  std::string oper;
  std::string time_slot;
  UserManager usermanager(user_file_name);
  TrainManager trainmanager(train_id_file_name, route_id_file_name,
                            station_id_file_name);
  TicketManager ticketmanager(ticket_id_file_name,
                              trainid_time_ticketid_file_name,
                              user_trainid_file_name, number_of_orders);
  while (std::cin >> time_slot >> oper) {
    getline(std::cin, input);
    std::cout << time_slot << " ";
    if (oper == "exit") {
      std::cout << "bye\n";
      break;
    } else if (oper == "add_user") {
      usermanager.add_user(input);
    } else if (oper == "login") {
      usermanager.login(input);
    } else if (oper == "logout") {
      usermanager.logout(input);
    } else if (oper == "query_profile") {
      usermanager.query_profile(input);
    } else if (oper == "modify_profile") {
      usermanager.modify_profile(input);
    } else if (oper == "add_train") {
      trainmanager.add_train(input);
    } else if (oper == "delete_train") {
      trainmanager.delete_train(input);
    } else if (oper == "release_train") {
      trainmanager.release_train(input);
    } else if (oper == "query_train") {
      trainmanager.query_train(input);
    } else if (oper == "query_ticket") {
      trainmanager.query_ticket(input);
    } else if (oper == "query_transfer") {
      trainmanager.query_transfer(input);
    } else if (oper == "buy_ticket") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager.ticket_parser(input);
      int no_use = 0;
      if (!usermanager.check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      if (trainmanager.check_ticket_enough(info)) {
        // ticket中buy
        // train中successful_purchase
      } else {
        if (info.find("-q") != info.end() && info["-q"] == "true") {
          // pending
        } else {
          std::cout << "-1\n";
        }
      }
    } else if (oper == "query_order") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager.ticket_parser(input);
      int no_use = 0;
      if (!usermanager.check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      ticketmanager.query_order(info);
    } else if (oper == "refund_ticket") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager.ticket_parser(input);
      int no_use = 0;
      if (!usermanager.check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      bool is_successful = 1;
      bool is_pending = 0;
      ticketmanager.refund_ticket(info, is_successful, is_pending);
      if (!is_successful) {
        std::cout << "-1\n";
      }
      if (!is_pending) {
        // realize refund in the train ticket
        // trainmanager.refund_ticket(const char *id, const int &num, const int
        // &departure_day, const int &start_station, const int &end_station)
      }
    } else if (oper == "clean") {
      usermanager.clean(user_file_name);
      trainmanager.clean(train_id_file_name, route_id_file_name,
                         station_id_file_name);
      ticketmanager.clean(ticket_id_file_name, trainid_time_ticketid_file_name,
                          user_trainid_file_name, number_of_orders);
    }
  }
  return 0;
}