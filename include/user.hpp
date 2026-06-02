#pragma once
#include "bpt.hpp"
#include "map.hpp"
#include "vector.hpp"
#include <cstring>

class UserManager {
  struct User {
    char username[21]{};
    char password[31];
    char name[16];
    char mail[31];
    int privilege;

    bool operator<(const User &other) {
      return (strcmp(username, other.username) < 0);
    }

    bool operator==(const User &other) {
      return strcmp(username, other.username) == 0;
    }
  };

  sjtu::vector<User> user_stack;

  BPT<User> user_info;

  bool check_username(const std::string &);

  bool check_password(const std::string &);

  bool check_name(const std::string &);

  bool check_mail(const std::string &);

  bool check_privilege(const std::string &);

  sjtu::map<std::string, std::string> user_parser(const std::string &input);

  void print_user(const User *cur_user);

public:
  UserManager(const std::string &file_name);

  bool check_login(const std::string &, int &);

  int add_user(const std::string &oper);

  int login(const std::string &oper);

  int logout(const std::string &oper);

  int query_profile(const std::string &oper);

  int modify_profile(const std::string &oper);

  std::string get_cur_user();

  void clean(const std::string &);
};