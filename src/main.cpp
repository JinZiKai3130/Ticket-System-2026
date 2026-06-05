#include "../include/ticket.hpp"
#include "../include/train.hpp"
#include "../include/user.hpp"
#include <iostream>
#include <limits>

const std::string user_file_name = "username_user";
const std::string train_id_file_name = "id_Train";
const std::string train_data_file_name = "train_data";
const std::string route_id_file_name = "route_id";
const std::string station_id_file_name = "station_id";
const std::string ticket_id_file_name = "id_Ticket";
const std::string trainid_time_ticketid_file_name = "pending_list";
const std::string user_trainid_file_name = "username_trainid";
const std::string number_of_orders = "order_count";

std::string input;
std::string oper;
std::string time_slot;

int main() {
  auto *usermanager = new UserManager(user_file_name);
  auto *trainmanager =
      new TrainManager(train_id_file_name, route_id_file_name,
                       station_id_file_name, train_data_file_name);
  auto *ticketmanager =
      new TicketManager(ticket_id_file_name, trainid_time_ticketid_file_name,
                        user_trainid_file_name, number_of_orders);
  while (std::cin >> time_slot >> oper) {
    // std::cout << time_slot << " " << oper << "\n";
    input.clear();
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    getline(std::cin, input, '\n');
    // std::cout << "input = " << input << "\n";
    std::cout << time_slot << " ";
    if (oper == "exit") {
      std::cout << "bye\n";
      break;
    } else if (oper == "add_user") {
      std::cout << usermanager->add_user(input) << "\n";
    } else if (oper == "login") {
      std::cout << usermanager->login(input) << "\n";
    } else if (oper == "logout") {
      std::cout << usermanager->logout(input) << "\n";
    } else if (oper == "query_profile") {
      if (usermanager->query_profile(input) == -1) {
        std::cout << "-1\n";
      }
    } else if (oper == "modify_profile") {
      if (usermanager->modify_profile(input) == -1) {
        std::cout << "-1\n";
      }
    } else if (oper == "add_train") {
      std::cout << trainmanager->add_train(input) << "\n";
    } else if (oper == "delete_train") {
      std::cout << trainmanager->delete_train(input) << "\n";
    } else if (oper == "release_train") {
      std::cout << trainmanager->release_train(input) << "\n";
    } else if (oper == "query_train") {
      if (trainmanager->query_train(input) == -1) {
        std::cout << "-1\n";
      }
    } else if (oper == "query_ticket") {
      trainmanager->query_ticket(input);
      // std::cout << "getout here\n";
    } else if (oper == "query_transfer") {
      if (trainmanager->query_transfer(input) == -1) {
        std::cout << 0 << "\n";
      }
    } else if (oper == "buy_ticket") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager->ticket_parser(input);
      int no_use = 0;
      if (!usermanager->check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      int departure_time_index = 0;
      int departure_time = 0;
      int arrival_time = 0;
      int total_price = 0;
      int start_station_index = 0;
      int end_station_index = 0;
      int flag = trainmanager->check_ticket_enough(
          info, departure_time, arrival_time, total_price, departure_time_index,
          start_station_index, end_station_index);
      if (flag == 1) {
        // ticket中buy
        ticketmanager->buy_ticket(info, departure_time, arrival_time,
                                  total_price, departure_time_index);
        // train中successful_purchase
        trainmanager->successful_ticket_purchase(
            info["-i"].c_str(), std::stoi(info["-n"]), departure_time_index,
            start_station_index, end_station_index);
        std::cout << total_price * std::stoi(info["-n"]) << "\n";
      } else {
        if (flag == 0 && info.find("-q") != info.end() &&
            info["-q"] == "true") {
          // pending
          ticketmanager->pend_ticket(info, departure_time, arrival_time,
                                     total_price, departure_time_index);
          std::cout << "queue\n";
        } else {
          std::cout << "-1\n";
        }
      }
    } else if (oper == "query_order") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager->ticket_parser(input);
      int no_use = 0;
      if (!usermanager->check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      ticketmanager->query_order(info);
    } else if (oper == "refund_ticket") {
      sjtu::map<std::string, std::string> info;
      info = ticketmanager->ticket_parser(input);
      int no_use = 0;
      if (!usermanager->check_login(info["-u"].c_str(), no_use)) {
        std::cout << "-1\n";
        continue;
      }
      bool is_successful = 1;
      bool is_pending = 0;
      std::string cur_train_id;
      int ticket_num = 0;
      int departure_time_index = 0;
      std::string departure_station_name;
      std::string arrival_station_name;
      ticketmanager->refund_ticket(
          info, is_successful, is_pending, cur_train_id, ticket_num,
          departure_time_index, departure_station_name, arrival_station_name);
      if (!is_successful) {
        std::cout << "-1\n";
        continue;
      }
      if (!is_pending) {
        // realize refund in the train ticket
        trainmanager->refund_ticket(
            cur_train_id.c_str(), ticket_num, departure_time_index,
            departure_station_name, arrival_station_name);
      }
      std::cout << "0\n";
    } else if (oper == "clean") {
      usermanager->clean(user_file_name);
      trainmanager->clean(train_id_file_name, route_id_file_name,
                          station_id_file_name, train_data_file_name);
      ticketmanager->clean(ticket_id_file_name, trainid_time_ticketid_file_name,
                           user_trainid_file_name, number_of_orders);
      std::cout << "0\n";
    }
  }
  delete usermanager;
  delete trainmanager;
  delete ticketmanager;
  return 0;
}