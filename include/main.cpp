#include "TrainSystem.hpp"
#include "UserSystem.hpp"
#include "bptree.hpp"
#include "utils.hpp"

#include "TicketSystem.hpp"

sjtu::TicketSystem KS;
sjtu::UserSystem& US = KS.US;
sjtu::TrainSystem& TS = KS.TS;
using sjtu::SplitString;

// 直接把parser写在这里算了

int main() {
  freopen64("in.in", "r", stdin);
  freopen64("out.out", "w", stdout);
  std::ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  sjtu::vector<string> tokens;
  string input;
  while (getline(cin, input)) {
    SplitString(tokens, input, ' ');

    if (tokens[0] == "[21589]")
      cout << "";
    if (tokens[0] == "[18220]")
      cout << "";

    cout << tokens[0] << ' ';  // timestamp
    string& cmd = tokens[1];
    if (cmd == "exit") {
      cout << "bye\n";
      break;
    } else if (cmd == "clear") {
      US.Clear();
      TS.Clear();
      KS.Clear();
    } else if (cmd == "add_user") {
      string item, curuser, user, password, name, mail;
      int privilege;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'c':
            curuser = tokens[i + 1];
            break;
          case 'u':
            user = tokens[i + 1];
            break;
          case 'p':
            password = tokens[i + 1];
            break;
          case 'n':
            name = tokens[i + 1];
            break;
          case 'm':
            mail = tokens[i + 1];
            break;
          case 'g':
            privilege = stoi(tokens[i + 1]);
            break;
          default:
            throw;
            break;
        }
      }
      US.AddUser(curuser, user, password, name, mail, privilege);
    } else if (cmd == "login") {
      string item, user, password;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'u':
            user = tokens[i + 1];
            break;
          case 'p':
            password = tokens[i + 1];
            break;
          default:
            throw;
            break;
        }
      }
      US.Login(user, password);
    } else if (cmd == "logout") {
      string curuser = tokens[3];
      US.Logout(curuser);
    } else if (cmd == "query_profile") {
      string item, curuser, user;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'c':
            curuser = tokens[i + 1];
            break;
          case 'u':
            user = tokens[i + 1];
            break;
          default:
            throw;
            break;
        }
      }
      US.QueryProfile(curuser, user);
    } else if (cmd == "modify_profile") {
      string item, curuser, user, password, name, mail;
      int privilege = -1;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'c':
            curuser = tokens[i + 1];
            break;
          case 'u':
            user = tokens[i + 1];
            break;
          case 'p':
            password = tokens[i + 1];
            break;
          case 'n':
            name = tokens[i + 1];
            break;
          case 'm':
            mail = tokens[i + 1];
            break;
          case 'g':
            privilege = stoi(tokens[i + 1]);
            break;
          default:
            throw;
            break;
        }
      }
      US.ModifyProfile(curuser, user, password, name, mail, privilege);
    } else if (cmd == "add_train") {
      string item, trainID, stations, prices, starttime, traveltimes, stopovertimes, salesdate;
      int stationnum, seatnum;
      char type;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'i':
            trainID = tokens[i + 1];
            break;
          case 'n':
            stationnum = stoi(tokens[i + 1]);
            break;
          case 'm':
            seatnum = stoi(tokens[i + 1]);
            break;
          case 's':
            stations = tokens[i + 1];
            break;
          case 'p':
            prices = tokens[i + 1];
            break;
          case 'x':
            starttime = tokens[i + 1];
            break;
          case 't':
            traveltimes = tokens[i + 1];
            break;
          case 'o':
            stopovertimes = tokens[i + 1];
            break;
          case 'd':
            salesdate = tokens[i + 1];
            break;
          case 'y':
            type = tokens[i + 1][0];
            break;
          default:
            break;
        }
      }
      TS.AddTrain(trainID, stationnum, seatnum, stations, prices, starttime, traveltimes, stopovertimes, salesdate, type);
    } else if (cmd == "delete_train") {
      string trainID = tokens[3];
      TS.DeleteTrain(trainID);
    } else if (cmd == "release_train") {
      string trainID = tokens[3];
      TS.ReleaseTrain(trainID);
    } else if (cmd == "query_train") {
      string item, trainID, date;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'i':
            trainID = tokens[i + 1];
            break;
          case 'd':
            date = tokens[i + 1];
            break;
          default:
            break;
        }
      }
      TS.QueryTrain(trainID, date);
    } else if (cmd == "query_ticket") {
      string item, date, from, to, tp;
      sjtu::SortType type;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'd':
            date = tokens[i + 1];
            break;
          case 's':
            from = tokens[i + 1];
            break;
          case 't':
            to = tokens[i + 1];
            break;
          case 'p':
            tp = tokens[i + 1];
            if (tp == "time")
              type = sjtu::TIME;
            else
              type = sjtu::COST;
            break;
        }
      }
      KS.QueryTicket(from, to, date, type);
    } else if (cmd == "query_transfer") {
      string item, date, from, to, tp;
      sjtu::SortType type;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'd':
            date = tokens[i + 1];
            break;
          case 's':
            from = tokens[i + 1];
            break;
          case 't':
            to = tokens[i + 1];
            break;
          case 'p':
            tp = tokens[i + 1];
            if (tp == "time")
              type = sjtu::TIME;
            else
              type = sjtu::COST;
            break;
        }
      }
      KS.QueryTransfer(from, to, date, type);
    } else if (cmd == "buy_ticket") {
      string item, user, trainID, date, from, to, tp;
      int num;
      bool queue = false;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'u':
            user = tokens[i + 1];
            break;
          case 'i':
            trainID = tokens[i + 1];
            break;
          case 'd':
            date = tokens[i + 1];
            break;
          case 'n':
            num = stoi(tokens[i + 1]);
            break;
          case 'f':
            from = tokens[i + 1];
            break;
          case 't':
            to = tokens[i + 1];
            break;
          case 'q':
            tp = tokens[i + 1];
            if (tp == "true")
              queue = true;
            else
              queue = false;
            break;
        }
      }
      KS.BuyTicket(user, trainID, date, from, to, num, queue);
    } else if (cmd == "query_order") {
      string user = tokens[3];
      KS.QueryOrder(user);
    } else if (cmd == "refund_ticket") {
      string item, user;
      int num;
      for (int i = 2; i < tokens.size(); i += 2) {
        item = tokens[i];
        switch (item[1]) {
          case 'u':
            user = tokens[i + 1];
            break;
          case 'n':
            num = stoi(tokens[i + 1]);
            break;
        }
      }
      KS.RefundTicket(user, num);
    } else
      throw;
  }
  return 0;
}