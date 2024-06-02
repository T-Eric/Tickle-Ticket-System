#ifndef SJTU_TICKETSYSTEM_TRAIN_HPP
#define SJTU_TICKETSYSTEM_TRAIN_HPP

#include "Calendar.hpp"
#include "bptree.hpp"
#include "utils.hpp"

namespace sjtu {
/*
先思考：
1.买票时日期为从这里出发的日期。由于sale时间段里每天发一辆车，故
*/

/*
火车特性：
1. saleDate里每天发一辆车，等效为几乎每天都有车
*/
struct Train {
  // 只与火车本身相关的信息
  char released;  // bool，发布与否
  ID trainID;     // 唯一标识
  char type;      // 列车类型
  int seatNum;    // 座位数，座位的覆盖效果左闭右开
  // 与火车站有关的信息
  int stationNum;        // 【2,100】
  String stations[100];  // 车站名
  int prices[100];       // 前缀和，[0]=0，第i项表示第0站到第i站的耗费钱数和
  int seats[94][100];    // 94表示6-8月最多有94天，第[i,j]项表示距离第一次发车时间为i天（第一天为0）时第j站的余票，终点站没有票
  // 时间信息
  // Time startTime;          // 每天都会发一辆车，发车时间
  // int travelTimes[100];    // 前缀和，[0]=0，第i项表示第0站到第i站的路途时间
  // int stopoverTimes[100];  // 列车在第i站停留的时间，[0]=0，可能设计为前缀和
  // 暂时采用了设计2：直接把每一站出发时间和到达时间算出来不是更好？
  Time departTimes[100];  // 从第i站出发的时间，共n-1项
  Time arriveTimes[100];  // 到达第i站的时间，[0]=0，共n-1项
  Date salesDate[2];      // 发售日期区间

  Train() = default;
  // 由于东西太多，就不给你赋值构造了，有点难受
  Train(const Train& other) {
    trainID = other.trainID;
    released = other.released;
    type = other.type;
    seatNum = other.seatNum;
    stationNum = other.stationNum;
    for (int i = 0; i < stationNum; i++) {
      stations[i] = other.stations[i];
      prices[i] = other.prices[i];
      // travelTimes[i] = other.travelTimes[i];
      // stopoverTimes[i] = other.stopoverTimes[i];
      departTimes[i] = other.departTimes[i];
      arriveTimes[i] = other.arriveTimes[i];
    }
    // startTime = other.startTime;
    for (int i = 0; i < 2; i++)
      salesDate[i] = other.salesDate[i];
    for (int i = 0; i < salesDate[1] - salesDate[0] + 1; ++i)
      for (int j = 0; i < stationNum - 1; ++j)
        seats[i][j] = seatNum;
  }
  Train& operator=(const Train& other) {
    if (this != &other) {
      trainID = other.trainID;
      released = other.released;
      type = other.type;
      seatNum = other.seatNum;
      stationNum = other.stationNum;
      for (int i = 0; i < stationNum; i++) {
        stations[i] = other.stations[i];
        prices[i] = other.prices[i];
        // travelTimes[i] = other.travelTimes[i];
        // stopoverTimes[i] = other.stopoverTimes[i];
        departTimes[i] = other.departTimes[i];
        arriveTimes[i] = other.arriveTimes[i];
      }
      // startTime = other.startTime;
      for (int i = 0; i < 2; i++)
        salesDate[i] = other.salesDate[i];
      for (int i = 0; i < salesDate[1] - salesDate[0] + 1; ++i)
        for (int j = 0; i < stationNum - 1; ++j)
          seats[i][j] = seatNum;
    }
    return *this;
  }
  ~Train() = default;

  // friend std::ostream& operator<<(std::ostream& os, const Train& t) {
  //   os<<t.trainID<<' '<<t.type<<'\n';
  //   //第一站，日期为starttime+saleDate
  //   cout<<t.stations[0]<<" xx-xx xx:xx -> "<<t.salesDate[0]<<' '<<t.startTime<<" 0 "<<t.prices[0];
  // }
};

class TrainSystem {
  friend class UserSystem;
  friend class TicketSystem;

 private:
  int siz = 0;  // 总车数，包括删掉的
  int head = sizeof(int);

  sjtu::BPTree<ID, int> trainIndex;
  sjtu::BPTree<String, Element<int, int> > stationIndex;
  std::fstream tfile;  // 存储真实数据，暂时不知道要不要给station也加一个
  const string tfilename = "TrainData.dat";

  vector<int> res;
  vector<string> tokens;
  vector<string> anothertokens;

  // int empty[501];  // 开一个500大小的空间回收
  // int frontpos;    // 假如empty用满了，直接从frontpos取

  void ReadProfile(int pos, Train& ret) {
    tfile.seekg(head + pos * sizeof(Train));
    tfile.read(reinterpret_cast<char*>(&ret), sizeof(Train));
  }
  void WriteProfile(int pos, const Train& up) {
    tfile.seekp(head + pos * sizeof(Train));
    tfile.write(reinterpret_cast<const char*>(&up), sizeof(Train));
  }

  // 查询是否已发布
  bool Released(int pos) {
    static char ch;
    tfile.seekg(head + pos * sizeof(Train));
    tfile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
    return ch != 0;
  }
  // 改变发布内容
  void ReviseRelease(int pos, bool releaseit = true) {
    char ch = releaseit;
    tfile.seekp(head + pos * sizeof(Train));
    tfile.write(reinterpret_cast<const char*>(&ch), sizeof(ch));
  }

  // // 由于要空间回收，给出一个位置
  // int GetPos() {
  //   if (!empty[0] || empty[0] == 500) {
  //     // 返回一个最前端没用过的位置
  //     int ret = frontpos;
  //     frontpos += sizeof(Train);
  //     return ret;
  //   }
  //   // 可以从回收站里面拉一个
  //   return empty[empty[0]--];
  // }
  // inline void Recycle(int pos) {
  //   if (empty[0] < 500)
  //     empty[++empty[0]] = pos;
  // }

 public:
  explicit TrainSystem()
      : trainIndex("TrainIndex.dat"), stationIndex("StationIndex.dat") {
    tfile.open(tfilename, std::ios::in | std::ios::out | std::ios::binary);
    if (!tfile.is_open()) {
      tfile.open(tfilename, std::ios::out);
      tfile.close();
      tfile.open(tfilename, std::ios::in | std::ios::out | std::ios::binary);
      siz = 0;
      tfile.seekp(0);
      tfile.write(reinterpret_cast<const char*>(&siz), sizeof(int));
      // tfile.write(reinterpret_cast<const char*>(&frontpos), sizeof(frontpos));
      // memset(empty, 0, sizeof(empty));
      // tfile.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
    } else {
      tfile.seekg(0);
      tfile.read(reinterpret_cast<char*>(&siz), sizeof(int));
      // tfile.read(reinterpret_cast<char*>(&frontpos), sizeof(frontpos));
      // tfile.read(reinterpret_cast<char*>(&empty), sizeof(empty));
    }
  }
  ~TrainSystem() {
    tfile.seekp(0);
    tfile.write(reinterpret_cast<const char*>(&siz), sizeof(siz));
    // tfile.write(reinterpret_cast<const char*>(&frontpos), sizeof(frontpos));
    // tfile.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
    tfile.close();
  }

  /*
加车，记得写拆分，尽量不用正则
* 如果火车只有两站，stopoverTimes为下划线，其他的都以'|'分割
* 已经存在自然添加失败，加入后release=false
* input:略
* return:成功与否
*/
  bool AddTrain(const string& id,
                int stationnum,
                int seatnum,
                const string& stations,
                const string& prices,
                const string& starttime,
                const string& traveltimes,
                const string& stopovertimes,
                const string& salesdate,
                const char& type) {
    // 先找是不是已经有了
    trainIndex.Find(id.c_str(), res);
    if (!res.empty()) {
      cout << "-1\n";
      return false;
    }

    Train tr;
    tr.released = 0;
    tr.trainID = id;
    tr.type = type;
    tr.seatNum = seatnum;
    tr.stationNum = stationnum;

    SplitString(tokens, stations);
    for (int i = 0; i < stationnum; ++i)
      tr.stations[i] = tokens[i];

    SplitString(tokens, prices);
    tr.prices[0] = 0;
    for (int i = 0; i < stationnum - 1; ++i)
      tr.prices[i + 1] = tr.prices[i] + stoi(tokens[i]);
    tr.departTimes[0] = starttime;
    SplitString(tokens, traveltimes);
    if (stopovertimes != "_")
      SplitString(anothertokens, stopovertimes);
    tr.arriveTimes[1] = tr.departTimes[0] + stoi(tokens[0]);
    for (int i = 1; i < stationnum - 1; ++i) {
      tr.departTimes[i] = tr.arriveTimes[i] + stoi(anothertokens[i - 1]);
      tr.arriveTimes[i + 1] = tr.departTimes[i] + stoi(tokens[i]);
    }

    SplitString(tokens, salesdate);
    tr.salesDate[0] = tokens[0], tr.salesDate[1] = tokens[1];
    int p = (tr.salesDate[1] - tr.salesDate[0]) + 1;
    // 别忘了设置每一站的座位
    for (int i = 0; i < tr.salesDate[1] - tr.salesDate[0] + 1; ++i)
      for (int j = 0; j < stationnum - 1; ++j)
        tr.seats[i][j] = seatnum;  // 最后一站没有票哦

    // 可以写入了
    WriteProfile(siz, tr);
    trainIndex.Insert(Element<ID, int>(id.c_str(), siz++));
    cout << "0\n";
    return true;
  }

  /*
  删掉一趟车，必须是未发布的
  */
  bool DeleteTrain(const string& id) {
    trainIndex.Find(id.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    if (Released(res[0])) {
      cout << "-1\n";
      return false;
    }
    trainIndex.Remove(Element<ID, int>(id.c_str(), res[0]));
    cout << "0\n";
    return true;
  }

  /*
  放出一辆车
  * 发布前的车次，不可发售车票，无法被 `query_ticket` 和 `query_transfer` 操作所查询到
  * 发布后的车次不可被删除
  */
  bool ReleaseTrain(const string& id) {
    trainIndex.Find(id.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }
    if (Released(res[0])) {
      cout << "-1\n";
      return false;
    }
    ReviseRelease(res[0]);
    static Train tr;
    ReadProfile(res[0], tr);
    for (int i = 0; i < tr.stationNum; ++i)
      stationIndex.Insert(Element(tr.stations[i], Element(res[0], i)));
    // 这一步存了这个站->这是第first个车次的第second个车站
    cout << "0\n";
    return true;
  }

  /*
  查询一辆车的信息
  * 格式：
  * ID type
  * (下面stationNum行)
  * 车站名stations[i] arrivingDateTime -> LeavingDateTime prices seats
  * 开始和结束的时间用xx-xx xx:xx表示，终点站剩余票数用x表示
  * 显然，第一行的价格是0
  input:ID,发车日期date
  return:成功与否
  干脆不做成返回string，而是我自己发算了
  */
  bool QueryTrain(const string& id, const string& dat) {
    // 在某一天发车，后面的启动时间貌似要直接算出来
    trainIndex.Find(id.c_str(), res);
    if (res.empty()) {
      cout << "-1\n";
      return false;
    }  // pos=res[0]
    static Train tr;
    static Date d;
    d = dat;
    ReadProfile(res[0], tr);
    int deltaday = dat - tr.salesDate[0];  // 用于seats

    if (d < tr.salesDate[0] || tr.salesDate[1] < d) {
      cout << "-1\n";
      return false;
    }

    cout << tr.trainID << ' ' << tr.type << '\n';
    cout << tr.stations[0] << " xx-xx xx:xx -> " << DateTime(d, tr.departTimes[0]) << ' ' << tr.prices[0] << ' ' << tr.seats[deltaday][0] << '\n';
    for (int i = 1; i < tr.stationNum - 1; ++i) {
      cout << tr.stations[i] << ' ' << DateTime(d, tr.arriveTimes[i]) << " -> " << DateTime(d, tr.departTimes[i]) << ' ' << tr.prices[i] << ' ' << tr.seats[deltaday][i] << '\n';
    }
    cout << tr.stations[tr.stationNum - 1] << ' ' << DateTime(d, tr.arriveTimes[tr.stationNum - 1]) << " -> xx-xx xx:xx " << tr.prices[tr.stationNum - 1] << " x" << '\n';
    return true;
  }

  void Clear() {
    siz = 0;
    trainIndex.Clear();
    stationIndex.Clear();
  }
};

}  // namespace sjtu

#endif  // SJTU_TICKETSYSTEM_TRAIN_HPP
