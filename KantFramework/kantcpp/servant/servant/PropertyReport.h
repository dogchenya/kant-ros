#ifndef _KANT_PROPERTY_REPORT_H_
#define _KANT_PROPERTY_REPORT_H_

#include "util/kt_lock.h"
//#include "util/kt_autoptr.h"
#include "util/kt_thread_mutex.h"
#include "util/kt_spin_lock.h"
#include <tuple>
#include <vector>
#include <string>

using namespace std;

#undef max
#undef min

namespace kant {

///////////////////////////////////////////////////////////////////////////////////
//
/**
 * 用户自定义属性统计基类
 */
class PropertyReport {
 public:
  /**
     * 设置该属性的服务名称，如果不设置则为当前服务
     */
  void setMasterName(const std::string& sMasterName) { _sMasterName = sMasterName; }

  /**
     * 获取该属性的服务名称
     */
  const std::string& getMasterName() const { return _sMasterName; }

 public:
  /**
     * 求和
     */
  class sum {
   public:
    sum() : _d(0) {}
    string get();
    string desc() { return "Sum"; }
    void set(int o) { _d += o; }

   protected:
    void clear() { _d = 0; }

   private:
    int _d;
  };

  /**
     * 求平均
     */
  class avg {
   public:
    avg() : _sum(0), _count(0) {}
    string desc() { return "Avg"; }
    string get();
    void set(int o) {
      _sum += o;
      ++_count;
    }

   protected:
    void clear() {
      _sum = 0;
      _count = 0;
    }

   private:
    int _sum;
    int _count;
  };

  /**
     * 分布
     */
  class distr {
   public:
    distr(){};
    distr(const vector<int>& range);
    string desc() { return "Distr"; }
    void set(int o);
    string get();

   protected:
    void clear() { _result.clear(); }

   private:
    vector<int> _range;
    vector<size_t> _result;
  };

  /**
     * 求最大值
     */
  class max {
   public:
    max() : _d(-9999999) {}
    string desc() { return "Max"; }
    string get();
    void set(int o) { _d < o ? _d = o : 1; }

   protected:
    void clear() { _d = 0; }

   private:
    int _d;
  };

  /**
     * 求最小值
     */
  class min {
   public:
    min() : _d(0) {}
    string desc() { return "Min"; }
    string get();
    void set(int o);

   protected:
    void clear() { _d = 0; }

   private:
    int _d;
  };

  /**
     * 计数
     */
  class count {
   public:
    count() : _d(0) {}
    string desc() { return "Count"; }
    string get();
    void set(int o) { _d++; }

   protected:
    void clear() { _d = 0; }

   private:
    int _d;
  };

 public:
  virtual void report(int iValue) = 0;
  virtual vector<pair<string, string>> get() = 0;

 protected:
  std::string _sMasterName;  //属性所属服务名称
};

typedef std::shared_ptr<PropertyReport> PropertyReportPtr;

///////////////////////////////////////////////////////////////////////////////////
//
/**
 * 具体的属性策略管理
 */

template <typename... Params>
class PropertyReportImp : public PropertyReport, public KT_ThreadMutex {
 public:
  using PropertyReportData = std::tuple<Params...>;

  PropertyReportImp(Params&&... args) : _propertyReportData(std::forward<Params>(args)...) {}

  // do NOT copy
  PropertyReportImp(const PropertyReportImp&) = delete;
  void operator=(const PropertyReportImp&) = delete;

  // but CAN move
  PropertyReportImp(PropertyReportImp&&) = default;
  PropertyReportImp& operator=(PropertyReportImp&&) = default;

  /**
    * 设置调用数据
    * @param iValue,值
    */
  void report(int iValue) override {
    KT_LockT<KT_ThreadMutex> lock(*this);
    Helper<std::tuple_size<decltype(_propertyReportData)>::value>::Report(*this, iValue);
  }

  /**
     * 获取属性信息
     *
     * @return vector<pair<string, string>>
     */
  vector<pair<string, string>> get() override {
    KT_LockT<KT_ThreadMutex> lock(*this);
    return Helper<std::tuple_size<decltype(_propertyReportData)>::value>::Get(*this);
  }

 private:
  // report helper
  template <int N, typename DUMMY = void>
  struct Helper {
    static void Report(PropertyReportImp<Params...>& pp, int iValue) {
      static_assert(N >= 1, "Obviously success");
      Helper<N - 1, DUMMY>::Report(pp, iValue);
      pp.template SetResult<N - 1>(iValue);
    }

    static std::vector<std::pair<std::string, std::string>> Get(PropertyReportImp<Params...>& pp) {
      static_assert(N >= 1, "Obviously success");

      std::vector<std::pair<std::string, std::string>> vs = Helper<N - 1, DUMMY>::Get(pp);

      vs.push_back({std::get<N - 1>(pp._propertyReportData).desc(), std::get<N - 1>(pp._propertyReportData).get()});
      return vs;
    }
  };

  template <typename DUMMY>
  struct Helper<0, DUMMY> {
    // base template
    static void Report(PropertyReportImp<Params...>&, int) {}

    static std::vector<std::pair<std::string, std::string>> Get(PropertyReportImp<Params...>&) {
      return std::vector<std::pair<std::string, std::string>>();
    }
  };

  template <int I>
  void SetResult(int iValue) {
    std::get<I>(_propertyReportData).set(iValue);
  }
  /**
     * 状态报告数据
     */
  PropertyReportData _propertyReportData;
};

}  // namespace kant

#endif
