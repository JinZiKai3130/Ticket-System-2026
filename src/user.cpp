#include "../include/user.hpp"
// #include "../include/map.hpp"
#include <sstream>

bool UserManager::check_login(const std::string &username, int &offset) {
  for (int i = 0; i < user_stack.size(); i++) {
    if (strcmp(user_stack[i].username, username.c_str()) == 0) {
      offset = i;
      return 1;
    }
  }
  return 0;
}

bool UserManager::check_username(const std::string &str) {
  if (str.empty() || str.size() > 20) {
    return false;
  }

  char first = str[0];
  if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))) {
    return false;
  }
  for (int i = 1; i < str.size(); ++i) {
    char ch = str[i];
    bool isLetter = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    bool isDigit = (ch >= '0' && ch <= '9');
    bool isUnderline = (ch == '_');

    if (!isLetter && !isDigit && !isUnderline) {
      return false;
    }
  }
  return true;
}

bool UserManager::check_password(const std::string &str) {
  if (str.size() < 1 || str.size() > 30) {
    return false;
  }
  for (char ch : str) {
    if (!(ch >= ' ' && ch <= '~')) { // 可能有问题，空格是否是可见字符
      return false;
    }
  }

  return true;
}

bool UserManager::check_name(const std::string &str) {
  int total_bytes = str.size();
  if (total_bytes < 6 || total_bytes > 15) {
    return false;
  }
  if (total_bytes % 3 != 0) {
    return false;
  }
  for (int i = 0; i < total_bytes; i += 3) {
    unsigned char byte1 = static_cast<unsigned char>(str[i]);
    unsigned char byte2 = static_cast<unsigned char>(str[i + 1]);
    unsigned char byte3 = static_cast<unsigned char>(str[i + 2]);

    // 首字节范围，来自豆包，224-239中不只有中文还有日文等
    bool check1 = (byte1 >= 228 && byte1 <= 233);
    bool check2 = (byte2 >= 128 && byte2 <= 191);
    bool check3 = (byte3 >= 128 && byte3 <= 191);

    if (!check1 || !check2 || !check3) {
      return false;
    }
  }
  return true;
}

bool UserManager::check_mail(const std::string &str) {
  if (str.empty() || str.size() > 30) {
    return false;
  }
  for (size_t i = 0; i < str.size(); ++i) {
    char ch = str[i];
    bool isUpper = (ch >= 'A' && ch <= 'Z');
    bool isLower = (ch >= 'a' && ch <= 'z');
    bool isDigit = (ch >= '0' && ch <= '9');
    bool isAt = (ch == '@');
    bool isDot = (ch == '.');
    if (!isUpper && !isLower && !isDigit && !isAt && !isDot) {
      return false;
    }
  }
  return true;
}

bool UserManager::check_privilege(const std::string &str) {
  int priv;
  try {
    priv = std::stoi(str);
  } catch (...) {
    return false;
  }
  if (priv >= 0 && priv <= 10) {
    return true;
  } else {
    return false;
  }
}

sjtu::map<std::string, std::string>
UserManager::user_parser(const std::string &input) {
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

void UserManager::print_user(const User *cur_user) {
  std::cout << cur_user->username << " " << cur_user->name << " "
            << cur_user->mail << " " << cur_user->privilege << "\n";
}

UserManager::UserManager(const std::string &file_name) {
  user_info.init(file_name);
  user_stack.clear();
}

int UserManager::add_user(const std::string &oper) {
  sjtu::map<std::string, std::string> info = user_parser(oper);
  // -c -u -p -n -m -g
  auto cur_user_iterator = info.find("-c");
  auto user_iterator = info.find("-u");
  auto password_iterator = info.find("-p");
  auto name_iterator = info.find("-n");
  auto mail_iterator = info.find("-m");
  auto privilege = info.find("-g");
  // 输入总体格式不符合
  if (info.size() != 6) {
    return -1;
  }
  if (cur_user_iterator == info.end() || user_iterator == info.end() ||
      password_iterator == info.end() || name_iterator == info.end() ||
      mail_iterator == info.end() || privilege == info.end()) {
    return -1;
  }
  // 每个小项目不符合
  if (!check_username(info["-u"]) || !check_password(info["-p"]) ||
      !check_name(info["-n"]) || !check_mail(info["-m"])) {
    return -1;
  }
  int new_priv;
  if (!user_info.empty()) { // 非空且login有问题或者privilege有问题
    int num;
    if ((!check_login(info["-c"], num) || !check_privilege(info["-g"]))) {
      return -1;
    }
    new_priv = std::stoi(info["-g"]);
    for (int i = 0; i < user_stack.size(); i++) {
      if (user_stack[i].username == info["-c"]) {
        if (user_stack[i].privilege <= new_priv) {
          return -1;
        } else {
          break;
        }
      }
    }
  } else {
    new_priv = 10;
  }

  // 首先，查找是否已有username，返回-1
  BPT<User>::index_value to_be_inserted;
  strcpy(to_be_inserted.index, info["-u"].c_str());
  strcpy(to_be_inserted.value.username, info["-u"].c_str());
  strcpy(to_be_inserted.value.mail, info["-m"].c_str());
  strcpy(to_be_inserted.value.name, info["-n"].c_str());
  strcpy(to_be_inserted.value.password, info["-p"].c_str());
  to_be_inserted.value.privilege = -1;
  User vec[3];
  int num = 0;
  user_info.findinterval(user_info.root, to_be_inserted, vec, num);
  if (num != 0) {
    return -1;
  }
  // 然后实现add，返回0
  to_be_inserted.value.privilege = new_priv;
  user_info.insert(to_be_inserted);
  return 0;
}

int UserManager::login(const std::string &oper) {
  sjtu::map<std::string, std::string> info = user_parser(oper);
  if (info.size() != 2) {
    return -1;
  }
  auto user_iterator = info.find("-u");
  auto password_iterator = info.find("-p");

  if (user_iterator == info.end() || password_iterator == info.end()) {
    return -1;
  }
  BPT<User>::index_value to_be_logged_in;
  User vec[3];
  int num = 0;
  user_info.findinterval(user_info.root, to_be_logged_in, vec, num);
  if (num != 1 ||
      vec[1].password !=
          info["-p"]) { // 如果当前没有这个用户，或者和password不匹配
    return -1;
  }
  user_stack.push_back(vec[1]);
  return 0;
}

int UserManager::logout(const std::string &oper) {
  sjtu::map<std::string, std::string> info = user_parser(oper);
  if (info.size() != 1) {
    return -1;
  }
  auto user_iterator = info.find("-u");
  if (user_iterator == info.end()) {
    return -1;
  }
  int offset = -1;
  if (!check_login(info["-u"], offset)) {
    return -1;
  } else {
    user_stack.erase(user_stack.begin() + offset);
    return 0;
  }
}

int UserManager::query_profile(const std::string &oper) {
  sjtu::map<std::string, std::string> info = user_parser(oper);
  if (info.size() != 2) {
    return -1;
  }
  auto cur_user_iterator = info.find("-c");
  auto user_iterator = info.find("-u");
  if (user_iterator == info.end() || cur_user_iterator == info.end()) {
    return -1;
  }
  int offset;
  if (!check_login(info["-c"], offset)) { // 未登录
    return -1;
  }

  User &cur_user = user_stack[offset];
  User vec[3];
  BPT<User>::index_value target;
  strcpy(target.index, info["-u"].c_str());
  strcpy(target.value.username, info["-u"].c_str());
  target.value.privilege = -1;
  int num = 0;
  user_info.findinterval(user_info.root, target, vec, num);
  if (num != 1) { // 不存在这个用户
    return -1;
  }
  if (cur_user.username == info["-u"] ||
      cur_user.privilege > vec[1].privilege) {
    print_user(&cur_user);
    return 0;
  }
  return -1;
}

int UserManager::modify_profile(const std::string &oper) {
  sjtu::map<std::string, std::string> info = user_parser(oper);
  if (info.size() < 2 || info.size() > 6) {
    return -1;
  }
  auto cur_user_iterator = info.find("-c");
  auto user_iterator = info.find("-u");
  auto password_iterator = info.find("-p");
  auto name_iterator = info.find("-n");
  auto mail_iterator = info.find("-m");
  auto privilege = info.find("-g");
  if (user_iterator == info.end() || cur_user_iterator == info.end()) {
    return -1;
  }
  int offset;
  if (!check_login(info["-c"], offset)) { // 未登录
    return -1;
  }

  User &cur_user = user_stack[offset];
  User vec[3], copy_of_vec1;
  BPT<User>::index_value target;
  strcpy(target.index, info["-u"].c_str());
  strcpy(target.value.username, info["-u"].c_str());
  target.value.privilege = -1;
  int num = 0;
  user_info.findinterval(user_info.root, target, vec, num);
  if (num != 1) { // 不存在这个用户
    return -1;
  }
  copy_of_vec1 = vec[1];
  if (cur_user.username == info["-u"] ||
      cur_user.privilege > vec[1].privilege) { // 有编辑的权限
    if (password_iterator != info.end()) {     // 是否有修改单项
      if (check_password(info["-p"]))
        strcpy(vec[1].password, info["-p"].c_str());
      else
        return -1;
    }
    if (name_iterator != info.end()) {
      if (check_name(info["-n"]))
        strcpy(vec[1].name, info["-n"].c_str());
      else
        return -1;
    }
    if (mail_iterator != info.end()) {
      if (check_mail(info["-m"]))
        strcpy(vec[1].mail, info["-m"].c_str());
      else
        return -1;
    }
    if (privilege != info.end()) {
      if (check_privilege(info["-g"]))
        vec[1].privilege = std::stoi(info["-g"]);
      else
        return -1;
    }
    user_info.remove(
        BPT<User>::index_value(copy_of_vec1.username, copy_of_vec1));
    user_info.insert(BPT<User>::index_value(vec[1].username, vec[1]));
    return 0;
  }
  return -1;
}

std::string UserManager::get_cur_user() {
  return user_stack[user_stack.size() - 1].username;
}

void UserManager::clean(const std::string &str) { user_info.clean(str); }