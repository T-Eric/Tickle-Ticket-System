#ifndef SJTU_TICKETSYSTEM_USER_HPP
#define SJTU_TICKETSYSTEM_USER_HPP

#include "bptree.hpp"
#include "utils.hpp"

namespace sjtu {

struct User {
  ID userID;
  Word password;
  Word name;
  Word mail;
  int privilege;  //[0,10]

  User() = default;
  // 通过私有数据类型构造
  User(const ID& un, const Word& pw, const Word& nm, const Word& em, int p)
      : userID(un), password(pw), name(nm), mail(em), privilege(p) {}
  // 通过std::string构造
  User(const char* un, const char* pw, const char* nm, const char* em, int p)
      : userID(un), password(pw), name(nm), mail(em), privilege(p) {}
  User(const User& other)
      : userID(other.userID), password(other.password), name(other.name), mail(other.mail), privilege(other.privilege) {}
  User& operator=(const User& other) {
    if (this != &other) {
      userID = other.userID;
      password = other.password;
      name = other.name;
      mail = other.mail;
      privilege = other.privilege;
    }
    return *this;
  }
  ~User() = default;

  friend std::ostream& operator<<(std::ostream& os, const User& up) {
    os << up.userID << ' ' << up.name << ' ' << up.mail << ' ' << up.privilege;
    return os;
  }
};

class UserSystem {
  friend class TrainSystem;
  friend class TicketSystem;

  /*
  首先思考一下要实现什么东西
  add_user：添加用户，就没有然后了。由于不涉及删除，添加的
    用户直接用fstream输入到某个dat文件。
    甚至由于这个系统连人数都不需要，所以实现异常简单
  login和logout：登陆队列，队列不分先后（虽然也可以分先后）
    用自己写的Map当Set使用，这个是OlogN的，到时候可能改成一个哈希？
  query和modify：查询和修改资料，也没什么困难

  数据文件构成：int(总人数)+User*siz(所有数据)
  */
 private:
  int siz;  // 用户总数
  int head = sizeof(int);
  BPTree<ID, int> index;  // 索引库，用户ID-文件指针
  const string ufilename = "UserData.dat";
  std::fstream ufile;                // 用户数据出入口，存真实数据
  map<ID, pair<int, int> > onlines;  // 当前在线，用户ID-privilege-文件指针
  User tmp;
  vector<int> res;

  void ReadProfile(int pos, User& ret) {
    ufile.seekg(head + pos * sizeof(User));
    ufile.read(reinterpret_cast<char*>(&ret), sizeof(User));
  }
  void WriteProfile(int pos, const User& up) {
    ufile.seekp(head + pos * sizeof(User));
    ufile.write(reinterpret_cast<const char*>(&up), sizeof(User));
  }

 public:
  explicit UserSystem()
      : index("UserIndex.dat") {
    ufile.open(ufilename, std::ios::in | std::ios::out | std::ios::binary);
    if (!ufile.is_open()) {
      // 新建文件，此时siz一定是0
      ufile.open(ufilename, std::ios::out);
      ufile.close();
      ufile.open(ufilename, std::ios::in | std::ios::out | std::ios::binary);
      ufile.seekp(0);
      siz = 0;
      ufile.write(reinterpret_cast<const char*>(&siz), sizeof(int));
    } else {
      ufile.seekg(0);
      ufile.read(reinterpret_cast<char*>(&siz), sizeof(int));
    }
  }
  ~UserSystem() {
    ufile.seekp(0);
    ufile.write(reinterpret_cast<const char*>(&siz), sizeof(int));
    ufile.close();
  }

  /*
  添加用户，返回什么值其实无所谓
  * 操作者在线，新用户权限小于操作者权限，即成功
  * 第一个用户的创建不受上述规则限制(即siz=0时)
  input:操作者，创建用户ID，密码，姓名，邮箱，权限
  return:成功与否
  */
  bool AddUser(const string& cu, const string& un, const string& pw, const string& nm, const string& em, const int& p) {
    if (siz) {
      ID cur_user(cu.c_str());
      auto cit = onlines.find(cur_user);
      if (cit == onlines.end()) {
        cout << "-1\n";
        return false;
      }
      // 已登录
      if (cit->second.first <= p) {
        cout << "-1\n";
        return false;
      }
      // 权限足够
      User up(un.c_str(), pw.c_str(), nm.c_str(), em.c_str(), p);
      Element<ID, int> ins(up.userID, siz);
      index.Insert(ins);
      WriteProfile(siz++, up);
      cout << "0\n";
      return true;
    }
    // 第一用户
    User up(un.c_str(), pw.c_str(), nm.c_str(), em.c_str(), 10);
    Element<ID, int> ins(up.userID, siz);
    index.Insert(ins);
    WriteProfile(siz++, up);
    cout << "0\n";
    return true;
  }

  /*
  登录，将一个用户添加到onlines里
  * 用户不存在或密码错误，登录失败
  * 用户在线，登录失败
  input:ID，密码
  return:成功与否
  */
  bool Login(const string& un, const string& pw) {
    index.Find(un.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    // 有这个用户，res[0]为当前用户profile文件指针
    ReadProfile(res[0], tmp);
    if (onlines.find(tmp.userID) != onlines.end()) {
      cout << "-1\n";
      return false;
    }  // 已经在线
    if (strcmp(tmp.password.str, pw.c_str())) {
      cout << "-1\n";
      return false;
    }
    onlines[tmp.userID] = pair<int, int>(tmp.privilege, res[0]);
    cout << "0\n";
    return true;
  }

  /*
  登出，从onlines里移除
  * 用户不存在或者未登录，失败
  input:ID
  return:成功与否
  */
  bool Logout(const string& un) {
    index.Find(un.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    // 有这个用户
    ID id(un.c_str());
    auto it = onlines.find(id);
    if (it == onlines.end()) {
      cout << "-1\n";
      return false;
    }
    onlines.erase(it);
    {
      cout << "0\n";
      return true;
    }
  }

  /*
  用户查找任意一个人的profile，就是查找
  * cur_user已登录，权限大于待查找用户权限
  * 或者，查找自己的profile
  * input: cur_user,ID
  * return: 一行字符串，username,name,mailaddr,privilege
  */
  bool QueryProfile(const string& cu, const string& un) {
    // 是否登录？
    auto it = onlines.find(cu.c_str());
    if (it == onlines.end()) {
      cout << "-1\n";
      return false;
    }
    // cur_user在线
    if (cu == un) {
      // 自查
      ReadProfile(it->second.second, tmp);
      cout << tmp << '\n';
      return true;
    }
    index.Find(un.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    ReadProfile(res[0], tmp);
    if (it->second.first <= tmp.privilege) {
      cout << "-1\n";
      return false;
    }
    cout << tmp << '\n';
    return true;
  }

  /*
  用户修改某人信息，修改参数可选
  * cur_user已登录，权限大于待修改用户权限
  * 或者，修改自己的信息
  * 如果能修改，那么只能降级，无论别人和自己，听起来怪怪的？
  * input: cur_user,ID,(new pw),(new name),(new mail),(new privilege)
  * return: bool
  */
  bool ModifyProfile(const string& cu, const string& un, const string& pw, const string& nm, const string& em, const int& p) {
    auto it = onlines.find(cu.c_str());
    if (it == onlines.end()) {
      cout << "-1\n";
      return false;
    }
    // 在线
    if (cu == un) {
      // 自查
      if (p >= it->second.first) {
        cout << "-1\n";
        return false;
      }
      // 不会改高权限，可以放心改了
      ReadProfile(it->second.second, tmp);
      if (!pw.empty())
        tmp.password = pw.c_str();
      if (!nm.empty())
        tmp.name = nm.c_str();
      if (!em.empty())
        tmp.mail = em.c_str();
      if (p != -1)
        tmp.privilege = p;
      WriteProfile(it->second.second, tmp);
      cout << tmp << '\n';
      return true;
    }
    index.Find(un.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    // 已经存在
    ReadProfile(res[0], tmp);
    if (it->second.first <= tmp.privilege) {
      cout << "-1\n";
      return false;
    }
    if (!pw.empty())
      tmp.password = pw.c_str();
    if (!nm.empty())
      tmp.name = nm.c_str();
    if (!em.empty())
      tmp.mail = em.c_str();
    if (p != -1)
      tmp.privilege = p;
    WriteProfile(res[0], tmp);
    cout << tmp << '\n';
    return true;
  }

  // 检查一个用户是否已登录，登录则返回其用户文件指针，否则返回-1
  int Online(const ID& us) {
    auto it = onlines.find(us);
    if (it == onlines.end())
      return -1;
    return it->second.second;
  }

  void Clear() {
    siz = 0;
    onlines.clear();
    index.Clear();
  }
};

}  // namespace sjtu

#endif  // !SJTU_TICKETSYSTEM_USER_HPP
