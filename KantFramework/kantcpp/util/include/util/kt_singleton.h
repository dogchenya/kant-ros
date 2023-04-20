#ifndef _KT_SINGLETON_H_
#define _KT_SINGLETON_H_

#include "util/kt_monitor.h"
#include <cassert>
#include <cstdlib>
#include <mutex>

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file kt_singleton.h 
 * @brief  单件类 .  
 *  
 * 单件实现类
 * 
 * 没有实现对单件生命周期的管理,使用示例代码如下:
 * 
 * class A : public KT_Singleton<A, CreateStatic,  DefaultLifetime>
 * 
 * {
 * 
 *  public:
 * 
 *    A(){cout << "A" << endl;}
 * 
 *   ~A()
 * 
 *   {
 * 
 *     cout << "~A" << endl;
 * 
 * 
 *    }
 *
 *    void test(){cout << "test A" << endl;}
 * 
 * };
 * 对象的创建方式由CreatePolicy指定, 有如下方式:
 * 
 * CreateUsingNew: 在堆中采用new创建
 * 
 * CreateStatic`: 在栈中采用static创建
 * 
 * 对象生命周期管理由LifetimePolicy指定, 有如下方式:
 * 
 * DefaultLifetime:缺省声明周期管理
 * 
 *如果单件对象已经析够, 但是还有调用, 会触发异常 
 * 
 * PhoneixLifetime:不死生命周期
 * 
 * 如果单件对象已经析够, 但是还有调用, 会再创建一个
 * 
 * NoDestroyLifetime:不析够
 * 
 * 对象创建后不会调用析够函数析够, 通常采用实例中的方式就可以了
 *
 */
/////////////////////////////////////////////////

/**
 * @brief 定义CreatePolicy:定义对象创建策略
 */
template <typename T>
class CreateUsingNew {
 public:
  /**
     * @brief  创建.
     *  
     * @return T*
     */
  static T *create() { return new T; }

  /**
	 * @brief 释放. 
	 *  
     * @param t
     */
  static void destroy(T *t) { delete t; }
};

// template<typename T>
// class CreateUsingNew1
// {
// public:
//     /**
// 	 * @brief  创建.
// 	 *
//      * @return T*
//      */
//     static T* create()
//     {
//         return new T;
//     }

//     /**
// 	 * @brief 释放.
// 	 *
//      * @param t
//      */
//     static void destroy(T *t)
//     {
//         delete t;
//     }
// };

template <typename T>
class CreateStatic {
 public:
  /**
     * @brief   最大的空间
     */
  union MaxAlign {
    char t_[sizeof(T)];
    long double longDouble_;
  };

  /**
     * @brief   创建. 
     * 
     * @return T*
     */
  static T *create() {
    static MaxAlign t;
    return new (&t) T;
  }

  /**
     * @brief   释放. 
     *  
     * @param t
     */
  static void destroy(T *t) { t->~T(); }
};

template <typename T>
class CreateRealStatic {
 public:
  /**
	 * @brief   创建.
     *
     * @return T*
     */
  static T *create() {
    static T t;
    return &t;
  }

  /**
	 * @brief   释放.
	 *
     * @param t
     */
  static void destroy(T *t) {}
};

////////////////////////////////////////////////////////////////
/**
 * @brief 定义LifetimePolicy:定义对象的声明周期管理
 * 进程退出时销毁对象
 */
template <typename T>
class DefaultLifetime {
 public:
  static void deadReference() { throw std::logic_error("singleton object has dead."); }

  static void scheduleDestruction(T *, void (*pFun)()) { std::atexit(pFun); }
};

/**
 * @brief,
 *       对象被销毁后可以重生(比如log,全局任何时候都需要)
 *
 * @author jarod (7/29/2015)
 */
template <typename T>
class PhoneixLifetime {
 public:
  static void deadReference() { _bDestroyedOnce = true; }

  static void scheduleDestruction(T *, void (*pFun)()) {
    if (!_bDestroyedOnce) std::atexit(pFun);
  }

 private:
  static bool _bDestroyedOnce;
};
template <class T>
bool PhoneixLifetime<T>::_bDestroyedOnce = false;

/**
 * @brief 不做对象销毁
 *
 * @author jarod (7/29/2015)
 */
template <typename T>
struct NoDestroyLifetime {
  static void scheduleDestruction(T *, void (*)()) {}

  static void deadReference() {}
};

//////////////////////////////////////////////////////////////////////
// Singleton
template <typename T, template <typename> class CreatePolicy = CreateUsingNew,
          template <typename> class LifetimePolicy = DefaultLifetime>
class KT_Singleton {
 public:
  typedef T instance_type;
  typedef volatile T volatile_type;

  typedef CreatePolicy<T> TCreatePolicy;

  /**
     * @brief 获取实例
     * 
     * @return T*
     */
  static T *getInstance() {
    static std::mutex __mutex_singleton;

    auto sin = __pInstance.load();
    if (!sin) {
      std::lock_guard<std::mutex> myLock(__mutex_singleton);
      sin = __pInstance.load();
      if (!sin) {
        if (__destroyed) {
          LifetimePolicy<T>::deadReference();
          __destroyed = false;
        }

        sin = CreatePolicy<T>::create();
        __pInstance.store(sin);
        LifetimePolicy<T>::scheduleDestruction(__pInstance, &destroySingleton);
      }
    }

    return sin;
  }

  virtual ~KT_Singleton(){};

 protected:
  static void destroySingleton() {
    assert(!__destroyed);
    CreatePolicy<T>::destroy((T *)__pInstance);
    __pInstance = NULL;
    __destroyed = true;
  }

 protected:
  static atomic<T *> __pInstance;
  static bool __destroyed;

 protected:
  KT_Singleton() = default;
  KT_Singleton(const KT_Singleton &) = default;
  KT_Singleton &operator=(const KT_Singleton &) = default;
};

template <class T, template <class> class CreatePolicy, template <class> class LifetimePolicy>
bool KT_Singleton<T, CreatePolicy, LifetimePolicy>::__destroyed = false;

template <class T, template <class> class CreatePolicy, template <class> class LifetimePolicy>
atomic<T *> KT_Singleton<T, CreatePolicy, LifetimePolicy>::__pInstance = {nullptr};
}  // namespace kant

#endif
