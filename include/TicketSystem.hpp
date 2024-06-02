#ifndef SJTU_TICKETSYSTEM_TICKET_HPP
#define SJTU_TICKETSYSTEM_TICKET_HPP

#include "Calendar.hpp"
#include "TrainSystem.hpp"
#include "UserSystem.hpp"
#include "bptree.hpp"

namespace sjtu {

enum SortType {
  TIME = 0,
  COST
};
enum OrderType {
  SUCCESS = 0,
  QUEUE,
  REFUNDED
};

// 一趟直达车
struct DirectTravel {
  ID trainID;
  int compthing;  // 用来比较的事物
  int pos;        // 一开始的vec中的序号，相当于我们存储的value
  DirectTravel() = default;
  DirectTravel(ID trainID_, int compthing_, int pos_)
      : trainID(trainID_), compthing(compthing_), pos(pos_) {}
  DirectTravel& operator=(const DirectTravel& other) {
    trainID = other.trainID;
    compthing = other.compthing;
    pos = other.pos;
    return *this;
  }
};
bool comp1(const DirectTravel& lhs, const DirectTravel& rhs) {
  // 以compthing为第一关键字，trainID为第二关键字
  if (lhs.compthing == rhs.compthing)
    return lhs.trainID < rhs.trainID;
  return lhs.compthing < rhs.compthing;
}
// comp2用于query_transfer，由于只要最优解，做成了普通的函数
bool comp2(int price, int tim, const ID& id1, const ID& id2, int curprice, int curtim, const ID& curid1, const ID& curid2, SortType type) {
  if (type == COST) {
    if (price != curprice)
      return curprice < price;
    if (tim != curtim)
      return curtim < tim;
    if (id1 != curid1)
      return curid1 < id1;
    return curid2 < id2;
  }
  // time
  if (tim != curtim)
    return curtim < tim;
  if (price != curprice)
    return curprice < price;
  if (id1 != curid1)
    return curid1 < id1;
  return curid2 < id2;
}

// 订单
struct Order {
  // 为了方便写入状态，放在前面了
  char status;   // 0.成功 1.正在候补，未有票 2.已经退票
  int trainpos;  // 便于直接找到这个车进行退票时修改
  int deltaday;  // 减少重复计算
  int from, to;  // station在这趟车上的位置

  ID userID;  // 谁的订单
  ID trainID;
  String fromStation, toStation;
  DateTime startTime, stopTime;
  Date startDate;  // 这趟火车的真正发车日期
  int price;       // 总价格
  int buy;         // 购买的票数
  Order() = default;
};

class TicketSystem {
  friend class UserSystem;
  friend class TrainSystem;

 private:
  int siz = 0;  // 已经有几张订单
  int head = sizeof(int);
  BPTree<int, int> orderIndex;                // 用户信息-订单存储位置
  BPTree<Element<int, int>, int> queueIndex;  // 车次编号-相对发车日的日期-订单存储位置
  std::fstream ofile;                         // 存储订单
  const string& filename = "ticketData.dat";

  vector<Element<int, int> > from;
  vector<Element<int, int> > to;
  vector<int> res;

  void ReadOrder(int pos, Order& ret) {
    ofile.seekg(pos);
    ofile.read(reinterpret_cast<char*>(&ret), sizeof(Order));
  }
  void WriteOrder(int pos, const Order& order) {
    ofile.seekp(pos);
    ofile.write(reinterpret_cast<const char*>(&order), sizeof(Order));
  }

 public:
  TrainSystem TS;
  UserSystem US;
  TicketSystem()
      : orderIndex("orderIndex.dat"), queueIndex("queueIndex.dat") {
    ofile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!ofile.is_open()) {
      ofile.open(filename, std::ios::out);
      ofile.close();
      ofile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
      siz = 0;
      ofile.seekp(0);
      ofile.write(reinterpret_cast<const char*>(&siz), sizeof(int));
    } else {
      ofile.seekg(0);
      ofile.read(reinterpret_cast<char*>(&siz), sizeof(int));
    }
  }
  ~TicketSystem() {
    ofile.seekp(0);
    ofile.write(reinterpret_cast<const char*>(&siz), sizeof(int));
    ofile.close();
  }

  /*
  super-frequent!
  查找直达车票，直接分别用两个指针遍历，找到所有的共有车次
  检查【起点站的序号是否小于终点站，即车次方向对不对】
  以及【当天有没有车次】
  找到后存到新的vector里面，进行后续排序输出
  input:始发站，终点站，始发站出发日期，排序规则（true=time,false=）
  自己输出：trainID fromStation DateTime -> toStation DateTime
   */
  bool QueryTicket(const string& from_, const string& to_, const string& dat, SortType type = TIME) {
    // bpt的find返回的vector，内部元素一定是按照Element排序的，对两个vec直接双指针处理即可
    TS.stationIndex.Find(from_.c_str(), from);
    TS.stationIndex.Find(to_.c_str(), to);
    static Train tr;  // 当前目标车辆
    static Date d;
    d = dat;  // 列车从from出发日期
    // from和to中存了所有的【车站编号-第几个车站】
    vector<DirectTravel> travel;  // 用于排序
    vector<int> timeprice[2];     // 0=time,1=price，就不用判断了
    vector<int> seat;
    vector<DateTime> starttime;
    vector<DateTime> stoptime;
    int i = 0, j = 0;
    while (i < from.size() && j < to.size()) {
      if (from[i].key == to[j].key) {
        if (from[i].val < to[j].val) {
          TS.ReadProfile(from[i].key, tr);
          // 判断日期，如果始发日期在这一天之后，或最后一车出发日期在这一天之前，则没戏
          if (d < tr.salesDate[0] + tr.departTimes[from[i].val].days || tr.salesDate[1] + tr.departTimes[from[i].val].days < d) {
            ++i, ++j;
            continue;
          }
          /*
          记录：trainID，始发时间dt，抵达时间dt，旅途消耗的时间，累计价格，最大座位数
          */
          timeprice[0].push_back(tr.arriveTimes[to[j].val] - tr.departTimes[from[i].val]);
          timeprice[1].push_back(tr.prices[to[j].val] - tr.prices[from[i].val]);
          int seats = 2147483647, deltaday = d - tr.salesDate[0] - tr.departTimes[from[i].val].days;
          for (int k = from[i].val; k < to[j].val; ++k)
            seats = std::min(seats, tr.seats[deltaday][k]);  // 不需要考虑终点站的票数啊
          seat.push_back(seats);
          tr.arriveTimes[to[j].val].days -= tr.departTimes[from[i].val].days;
          tr.departTimes[from[i].val].days = 0;
          starttime.push_back(DateTime(d, tr.departTimes[from[i].val]));
          stoptime.push_back(DateTime(d, tr.arriveTimes[to[j].val]));
          travel.push_back(DirectTravel(tr.trainID, timeprice[type].back(), seat.size() - 1));
        }
        ++i, ++j;
      } else if (from[i].key < to[j].key)
        ++i;
      else
        ++j;
    }
    // 一辆都没有，直接返回
    if (travel.empty()) {
      cout << "0\n";
      return false;
    }
    int ppp = travel.size();
    // 排序，按照time或cost第一关键字，trainID第二关键字进行排序
    Sort(travel, comp1);  // 此时travel中的pos就是我们想要的
    cout << travel.size() << '\n';
    for (int i = 0; i < travel.size(); ++i) {
      int& p = travel[i].pos;
      cout << travel[i].trainID << ' ' << from_ << ' ' << starttime[p] << " -> " << to_ << ' ' << stoptime[p] << ' ' << timeprice[1][p] << ' ' << seat[p] << '\n';
    }
    return true;
  }

  /*
  查找换乘车票
  得到两个vector后遍历找到所有的共有车次，对遍历中每次得到的两个车次
  遍历所有的车站，找到所有的共有车站，作为中转站备选
  对于所有的中转站备选，使用查找直达车次的算法去找两趟车
  要保证前一趟车到站在后一趟车发车的前面，以及前一趟车必须在那一天发车
  input:始发站，终点站，始发站出发日期
  输出：买的两张车票
  */
  bool QueryTransfer(const string& from_, const string& to_, const string& dat, SortType type = TIME) {
    Date d(dat);
    TS.stationIndex.Find(from_.c_str(), from);
    TS.stationIndex.Find(to_.c_str(), to);

    int price = 2147483647, tim = 2147483647;
    ID id1("~"), id2("~");  // 最大的string
    int ans[2] = {-1, -1};  // 存储最终答案
    Element<int, int> stationID[2];
    Date realDate[2];  // 最终输出日期

    Train tr1, tr2;
    for (int i = 0; i < from.size(); ++i) {
      TS.ReadProfile(from[i].key, tr1);
      if (from[i].val == tr1.stationNum - 1)
        continue;  // 第一辆车的最后一站，有什么好坐的
      Date left = tr1.salesDate[0] + tr1.departTimes[from[i].val].days;
      Date right = tr1.salesDate[1] + tr1.departTimes[from[i].val].days;
      if (d < left || right < d)
        continue;  // 没有这个时间段的车
      for (int j = 0; j < to.size(); ++j) {
        if (from[i].key == to[j].key)
          continue;  // 一样的站就不要了
        TS.ReadProfile(to[j].key, tr2);

        // 开始检查共有车站，from往后找，to往前找
        for (int x = from[i].val; x < tr1.stationNum; ++x) {
          for (int y = 0; y < to[j].val; ++y) {
            if (tr1.stations[x] != tr2.stations[y])
              continue;  // 不一样的两站
            // 这个共有车站不能是from和to
            String stationName(from_.c_str());
            if (tr1.stations[x] == stationName || tr2.stations[y] == stationName)
              continue;
            DateTime fromcome(Date(d + (tr1.arriveTimes[x].days - tr1.departTimes[from[i].val].days)), Time(tr1.arriveTimes[x]) - tr1.arriveTimes[x].days * 1440);
            // 1车在这个点到达这一站，2车最后一趟出发不能比他晚
            DateTime togolast(tr2.salesDate[1], tr2.departTimes[y]);  // 2车最后一次发车
            if (togolast < fromcome)
              continue;
            // 此时可用这趟车
            int curtime, curprice;                                     // 全程时间、全程价格
            DateTime togofirst(tr2.salesDate[0], tr2.departTimes[y]);  // 2车第一次发车
            Date RealDate;
            if (togofirst <= fromcome) {
              // 可以当天走或第二天走，取决于时间
              Time t1 = tr1.arriveTimes[x];
              Time t2 = tr2.departTimes[y];
              t1.days = 0, t2.days = 0;  // 纯时间
              if (t1 <= t2) {
                // 当天可走
                curtime = (tr1.arriveTimes[x] - tr1.departTimes[from[i].val]) + (t2 - t1) + (tr2.arriveTimes[to[j].val] - tr2.departTimes[y]);
                RealDate = fromcome.date - tr2.departTimes[y].days;  // 减去，因为后面再构造要输出的时间时会加上这个days
              } else {                                               // 第二天走
                curtime = (tr1.arriveTimes[x] - tr1.departTimes[from[i].val]) + (1440 - (t2 - t1)) + (tr2.arriveTimes[to[j].val] - tr2.departTimes[y]);
                RealDate = fromcome.date - tr2.departTimes[y].days + 1;
              }  // RealDate计算貌似有一点问题
            } else {
              // 等第一班车
              curtime = (tr1.arriveTimes[x] - tr1.departTimes[from[i].val]) + (togofirst - fromcome) + (tr2.arriveTimes[to[j].val] - tr2.departTimes[y]);
              RealDate = tr2.salesDate[0];
            }
            curprice = tr1.prices[x] - tr1.prices[from[i].val] + tr2.prices[to[j].val] - tr2.prices[y];
            bool ret = comp2(price, tim, id1, id2, curprice, curtime, tr1.trainID, tr2.trainID, type);
            if (ret) {
              // 更优解
              price = curprice;
              tim = curtime;
              id1 = tr1.trainID;
              id2 = tr2.trainID;
              ans[0] = from[i].key, ans[1] = to[j].key;
              stationID[0] = Element(from[i].val, x);
              stationID[1] = Element(y, to[j].val);
              realDate[0] = d - tr1.departTimes[from[i].val].days;
              realDate[1] = RealDate;
            }
          }
        }
      }
    }
    if (price == 2147483647) {
      cout << "0\n";
      return false;
    }

    // 可以输出了
    static Train tr;
    for (int p = 0; p < 2; ++p) {
      TS.ReadProfile(ans[p], tr);
      int totalprice = tr.prices[stationID[p].val] - tr.prices[stationID[p].key];
      int maxseat = 2147483647;
      int deltaday = realDate[p] - tr.salesDate[0];
      for (int i = stationID[p].key; i < stationID[p].val; ++i)
        maxseat = std::min(maxseat, tr.seats[deltaday][i]);
      DateTime depart(realDate[p], tr.departTimes[stationID[p].key]);
      DateTime arrive(realDate[p], tr.arriveTimes[stationID[p].val]);
      cout << tr.trainID << ' ' << tr.stations[stationID[p].key] << ' ' << depart << " -> " << tr.stations[stationID[p].val] << ' ' << arrive << ' ' << totalprice << ' ' << maxseat << '\n';
    }
    return true;
  }

  /*
  买票，即找到对应的车次，将其区间减
  假如区间减做不到，那么考虑是否候补
  候补的话，放到queueIndex里面，但是怎么查找，不一定知道
  无论如何，放到orderIndex里面方便后面查找
  */
  bool BuyTicket(const string& us, const string& tn, const string& dat, const string& from_, const string& to_, int n, bool q) {
    static ID userID;
    userID = us.c_str();
    int userpos = US.Online(userID);
    if (userpos == -1) {
      cout << "-1\n";
      return false;
    }

    TS.trainIndex.Find(tn.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    Train tr;
    TS.ReadProfile(res[0], tr);
    if (tr.released == 0) {
      cout << "-1\n";
      return false;
    }
    // 检查余票
    int From = -1, To = -1;
    for (int i = 0; i < tr.stationNum; ++i) {
      if (tr.stations[i] == from_.c_str())
        From = i;
      if (tr.stations[i] == to_.c_str())
        To = i;
    }
    if (From == -1 || To == -1 || From >= To) {
      cout << "-1\n";
      return false;
    }
    Date d(dat);
    int deltaday = d - tr.salesDate[0] - tr.departTimes[From].days;
    if (deltaday < 0 || deltaday > tr.salesDate[1] - tr.salesDate[0]) {
      cout << "-1\n";
      return false;
    }  // 天数不符合车次性质
    // 检查余票
    if (tr.seatNum < n) {
      cout << "-1\n";
      return false;
    }
    bool enough = true;
    for (int i = From; i < To; ++i) {
      if (tr.seats[deltaday][i] < n) {
        enough = false;
        break;
      }
    }
    if (!enough && !q) {
      cout << "-1\n";
      return false;
    }  // 没有余票，不想候补

    Order order;
    order.userID = us.c_str();
    order.trainID = tn.c_str();
    order.trainpos = res[0];
    order.startDate = d - tr.departTimes[From].days;
    order.deltaday = deltaday;
    order.from = From;
    order.to = To;
    order.fromStation = tr.stations[From];
    order.toStation = tr.stations[To];
    order.startTime = DateTime(order.startDate, tr.departTimes[From]);
    order.stopTime = DateTime(order.startDate, tr.arriveTimes[To]);
    order.buy = n;
    order.price = tr.prices[To] - tr.prices[From];
    if (enough) {
      // 有余票，直接购买
      for (int i = From; i < To; ++i)
        tr.seats[deltaday][i] -= n;
      int totalprice = order.price * n;
      TS.WriteProfile(res[0], tr);
      order.status = SUCCESS;
      orderIndex.Insert(Element<int, int>(userpos, siz));
      ofile.seekp(head + (siz++) * sizeof(Order));
      ofile.write(reinterpret_cast<char*>(&order), sizeof(Order));
      cout << totalprice << '\n';
      return true;
    }
    // 候补
    order.status = QUEUE;
    orderIndex.Insert(Element<int, int>(userpos, siz));
    queueIndex.Insert(Element<Element<int, int>, int>(Element<int, int>(res[0], deltaday), siz));
    ofile.seekp(head + (siz++) * sizeof(Order));
    ofile.write(reinterpret_cast<const char*>(&order), sizeof(order));
    cout << "queue\n";
    return true;
  }

  /*
  查找某个用户购票信息，直接在orderIndex里面找即可
  */
  bool QueryOrder(const string& us) {
    static ID userID;
    userID = us.c_str();
    int userpos = US.Online(userID);
    if (userpos == -1) {
      cout << "-1\n";
      return false;
    }
    orderIndex.Find(userpos, res);
    if (res.empty()) {
      cout << "0\n";
      return true;
    }
    cout << res.size() << '\n';
    static Order order;
    // 从新到旧，因此反过来
    for (int i = res.size() - 1; i >= 0; --i) {
      ofile.seekg(head + res[i] * sizeof(Order));
      ofile.read(reinterpret_cast<char*>(&order), sizeof(Order));
      switch (order.status) {
        case SUCCESS:
          cout << "[success] ";
          break;
        case QUEUE:
          cout << "[pending] ";
          break;
        case REFUNDED:
          cout << "[refunded] ";
          break;
        default:
          throw;
      }
      cout << order.trainID << ' ' << order.fromStation << ' ' << order.startTime << " -> " << order.toStation << ' ' << order.stopTime << ' ' << order.price << ' ' << order.buy << '\n';
    }
    return true;
  }

  /*
  退票
  找到对应的车次，将其区间加
  对应的购票信息修改为refunded
  在候补队列中查找对应的订单，将其删掉（时间戳也是唯一标识，可以直接找到的）
  */
  bool RefundTicket(const string& us, int pos) {
    ID userID(us.c_str());
    int userpos = US.Online(userID);
    if (userpos == -1) {
      cout << "-1\n";
      return false;
    }
    orderIndex.Find(userpos, res);
    if ((int)res.size() < pos) {
      // 不足
      cout << "-1\n";
      return false;
    }
    int p = res.size() - pos;
    int prepos = res[p];
    Order order;
    ofile.seekg(head + prepos * sizeof(Order));
    ofile.read(reinterpret_cast<char*>(&order), sizeof(order));
    if (order.status == REFUNDED) {
      cout << "-1\n";
      return false;
    }
    if (order.status == QUEUE) {
      // 从候补队列中去掉
      order.status = REFUNDED;
      queueIndex.Remove(Element<Element<int, int>, int>(Element<int, int>(order.trainpos, order.deltaday), prepos));
      ofile.seekp(head + prepos * sizeof(Order));
      ofile.write(reinterpret_cast<const char*>(&(order.status)), sizeof(order.status));
      cout << "0\n";
      return true;
    }
    order.status = REFUNDED;
    // 已经买了票，要修改train的数据
    static Train tr;
    TS.ReadProfile(order.trainpos, tr);
    for (int i = order.from; i < order.to; ++i)
      tr.seats[order.deltaday][i] += order.buy;

    // 遍历候补队列，看看当天当车订单还有谁
    // 由于b+树顺序，返回的vec一定是按照下单顺序正序的，从头遍历
    queueIndex.Find(Element(order.trainpos, order.deltaday), res);
    Order tmp;
    for (int i = 0; i < res.size(); ++i) {
      ofile.seekg(head + res[i] * sizeof(Order));
      ofile.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
      // 这一步应读入状态-火车编号-特征天数-两个站在这趟车上的位置
      if (tmp.to < order.from || tmp.from > order.to)
        continue;          // 没有影响
      bool enough = true;  // 退票后是否有足够的票了
      for (int j = tmp.from; j < tmp.to; ++j) {
        if (tr.seats[order.deltaday][j] < tmp.buy) {
          enough = false;
          break;
        }
      }
      if (!enough)
        continue;  // 很遗憾

      for (int j = tmp.from; j < tmp.to; ++j)
        tr.seats[order.deltaday][j] -= tmp.buy;
      tmp.status = SUCCESS;
      ofile.seekp(head + res[i] * sizeof(Order));
      ofile.write(reinterpret_cast<const char*>(&(tmp.status)), sizeof(tmp.status));
      queueIndex.Remove(Element<Element<int, int>, int>(Element<int, int>(tmp.trainpos, tmp.deltaday), res[i]));
    }
    TS.WriteProfile(order.trainpos, tr);
    ofile.seekp(head + prepos * sizeof(Order));
    ofile.write(reinterpret_cast<const char*>(&(order.status)), sizeof(order.status));
    cout << "0\n";
    return true;
  }

  void Clear() {
    orderIndex.Clear();
    queueIndex.Clear();
    siz = 0;
  }
};

}  // namespace sjtu

#endif  // !SJTU_TICKETSYSTEM_TICKET_HPP
