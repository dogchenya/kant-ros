#ifndef _KC_THREAD_QUEUE_H_
#define _KC_THREAD_QUEUE_H_

#include <deque>
#include <vector>
#include <cassert>
#include <mutex>
#include <condition_variable>

using namespace std;

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file kt_thread_queue.h
 * @brief �̶߳�����.
 *  
 * @author jarodruan@upchina.com
 */

/////////////////////////////////////////////////
/**
 * @brief �̰߳�ȫ����
 */
template <typename T, typename D = deque<T>>
class KT_ThreadQueue {
 public:
  KT_ThreadQueue() : _size(0) {}

 public:
  typedef D queue_type;

  /**
	 * @brief ��ͷ����ȡ����, û���������쳣
	 *
	 * @param t
	 * @return bool: true, ��ȡ������, false, ������
	 */
  T front();

  /**
     * @brief ��ͷ����ȡ����, û��������ȴ�.
     *
     * @param t 
	 * @param millsecond(wait = trueʱ����Ч)  �����ȴ�ʱ��(ms)
	 *                    0 ��ʾ������ 
     * 					 -1 ���õȴ�
     * @param wait, �Ƿ�wait
     * @return bool: true, ��ȡ������, false, ������
     */
  bool pop_front(T& t, size_t millsecond = 0, bool wait = true);

  /**
	 * @brief ��ͷ����ȡ����.
	 *
	 * @return bool: true, ��ȡ������, false, ������
	 */
  bool pop_front();

  /**
     * @brief ֪ͨ�ȴ��ڶ���������̶߳��ѹ���
     */
  void notifyT();

  /**
	 * @brief �����ݵ����к��. 
	 *  
     * @param t
     */
  void push_back(const T& t, bool notify = true);

  /**
	 * @brief  �����ݵ����к��. 
	 *  
     * @param vt
     */
  void push_back(const queue_type& qt, bool notify = true);

  /**
	 * @brief  �����ݵ�����ǰ��. 
	 *  
     * @param t
     */
  void push_front(const T& t, bool notify = true);

  /**
	 * @brief  �����ݵ�����ǰ��. 
	 *  
     * @param vt
     */
  void push_front(const queue_type& qt, bool notify = true);

  /**
	 * @brief  ��������
	 *  
     * @param q
	 * @param millsecond(wait = trueʱ����Ч)  �����ȴ�ʱ��(ms)
	 *                   0 ��ʾ������ 
     * 					 -1 ���Ϊ�����õȴ�
     * @param �Ƿ�ȴ�������
     * @return �����ݷ���true, �����ݷ���false
     */
  bool swap(queue_type& q, size_t millsecond = 0, bool wait = true);

  /**
     * @brief  ���д�С.
     *
     * @return size_t ���д�С
     */
  size_t size() const;

  /**
     * @brief  ��ն���
     */
  void clear();

  /**
     * @brief  �Ƿ�����Ϊ��.
     *
     * @return bool Ϊ�շ���true�����򷵻�false
     */
  bool empty() const;

  /**
     * @brief  ��������ȴ�.
     *
     * @return bool �ǿշ���true����ʱ����false
     */
  bool wait(size_t millsecond);

 protected:
  KT_ThreadQueue(const KT_ThreadQueue&) = delete;
  KT_ThreadQueue(KT_ThreadQueue&&) = delete;
  KT_ThreadQueue& operator=(const KT_ThreadQueue&) = delete;
  KT_ThreadQueue& operator=(KT_ThreadQueue&&) = delete;

  bool hasNotify(size_t lockId) const { return lockId != _lockId; }

 protected:
  /**
     * ����
     */
  queue_type _queue;

  /**
     * ���г���
     */
  size_t _size;

  //��������
  std::condition_variable _cond;

  //��
  mutable std::mutex _mutex;

  //lockId, �ж������Ƿ��ѹ�
  size_t _lockId = 0;
};

template <typename T, typename D>
T KT_ThreadQueue<T, D>::front() {
  std::unique_lock<std::mutex> lock(_mutex);

  return _queue.front();
}

template <typename T, typename D>
bool KT_ThreadQueue<T, D>::pop_front(T& t, size_t millsecond, bool wait) {
  if (wait) {
    size_t lockId = _lockId;

    std::unique_lock<std::mutex> lock(_mutex);

    // �˴��ȴ����������� 1.��������; 2.���˻�����.
    // ��һ�������㶼�����Ƶȴ���������
    if (millsecond == (size_t)-1) {
      _cond.wait(lock, [&] { return !_queue.empty() || hasNotify(lockId); });
    } else if (millsecond > 0) {
      _cond.wait_for(lock, std::chrono::milliseconds(millsecond), [&] { return !_queue.empty() || hasNotify(lockId); });
    }

    // ��ʱ�����ݻ�û�� �� ��û��ʱ�ͱ�notify������, ֱ�ӷ���
    if (_queue.empty() || hasNotify(lockId)) {
      return false;
    }

    t = _queue.front();
    _queue.pop_front();
    assert(_size > 0);
    --_size;

    return true;
  } else {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_queue.empty()) {
      return false;
    }

    t = _queue.front();
    _queue.pop_front();
    assert(_size > 0);
    --_size;

    return true;
  }
}

template <typename T, typename D>
bool KT_ThreadQueue<T, D>::pop_front() {
  std::unique_lock<std::mutex> lock(_mutex);
  if (_queue.empty()) {
    return false;
  }

  _queue.pop_front();
  assert(_size > 0);
  --_size;

  return true;
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::notifyT() {
  std::unique_lock<std::mutex> lock(_mutex);
  ++_lockId;
  _cond.notify_all();
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::push_back(const T& t, bool notify) {
  if (notify) {
    std::unique_lock<std::mutex> lock(_mutex);

    _queue.push_back(t);
    ++_size;

    _cond.notify_one();
  } else {
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(t);
    ++_size;
  }
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::push_back(const queue_type& qt, bool notify) {
  if (notify) {
    std::unique_lock<std::mutex> lock(_mutex);

    typename queue_type::const_iterator it = qt.begin();
    typename queue_type::const_iterator itEnd = qt.end();
    while (it != itEnd) {
      _queue.push_back(*it);
      ++it;
      ++_size;
    }
    _cond.notify_all();
  } else {
    std::lock_guard<std::mutex> lock(_mutex);

    typename queue_type::const_iterator it = qt.begin();
    typename queue_type::const_iterator itEnd = qt.end();
    while (it != itEnd) {
      _queue.push_back(*it);
      ++it;
      ++_size;
    }
  }
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::push_front(const T& t, bool notify) {
  if (notify) {
    std::unique_lock<std::mutex> lock(_mutex);

    _cond.notify_one();

    _queue.push_front(t);

    ++_size;
  } else {
    std::lock_guard<std::mutex> lock(_mutex);

    _queue.push_front(t);

    ++_size;
  }
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::push_front(const queue_type& qt, bool notify) {
  if (notify) {
    std::unique_lock<std::mutex> lock(_mutex);

    typename queue_type::const_iterator it = qt.begin();
    typename queue_type::const_iterator itEnd = qt.end();
    while (it != itEnd) {
      _queue.push_front(*it);
      ++it;
      ++_size;
    }

    _cond.notify_all();
  } else {
    std::lock_guard<std::mutex> lock(_mutex);

    typename queue_type::const_iterator it = qt.begin();
    typename queue_type::const_iterator itEnd = qt.end();
    while (it != itEnd) {
      _queue.push_front(*it);
      ++it;
      ++_size;
    }
  }
}

template <typename T, typename D>
bool KT_ThreadQueue<T, D>::swap(queue_type& q, size_t millsecond, bool wait) {
  if (wait) {
    size_t lockId = _lockId;

    std::unique_lock<std::mutex> lock(_mutex);

    // �˴��ȴ����������� 1.��������; 2.notify����
    // ��һ�������㶼�����Ƶȴ���������
    if (millsecond == (size_t)-1) {
      _cond.wait(lock, [&] { return !_queue.empty() || hasNotify(lockId); });
    } else if (millsecond > 0) {
      _cond.wait_for(lock, std::chrono::milliseconds(millsecond), [&] { return !_queue.empty() || hasNotify(lockId); });
    }

    // ��ʱ�����ݻ�û�� �� ��û��ʱ�ͱ�notify������, ֱ�ӷ���
    if (_queue.empty() || hasNotify(lockId)) {
      return false;
    }

    q.swap(_queue);
    _size = _queue.size();

    return true;
  } else {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_queue.empty()) {
      return false;
    }

    q.swap(_queue);

    _size = _queue.size();

    return true;
  }
}

template <typename T, typename D>
size_t KT_ThreadQueue<T, D>::size() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _size;
}

template <typename T, typename D>
void KT_ThreadQueue<T, D>::clear() {
  std::lock_guard<std::mutex> lock(_mutex);
  _queue.clear();
  _size = 0;
}

template <typename T, typename D>
bool KT_ThreadQueue<T, D>::empty() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _queue.empty();
}

template <typename T, typename D>
bool KT_ThreadQueue<T, D>::wait(size_t millsecond) {
  size_t lockId = _lockId;

  std::unique_lock<std::mutex> lock(_mutex);

  if (_queue.empty()) {
    if (millsecond == 0) {
      return false;
    }
    if (millsecond == (size_t)-1) {
      _cond.wait(lock, [&] { return !_queue.empty() || hasNotify(lockId); });
      //            _cond.wait(lock);
    } else {
      //��ʱ��
      //	        _cond.wait_for(lock, std::chrono::milliseconds(millsecond), [&] { return !_queue.empty() || hasNotify(lockId); });

      return _cond.wait_for(lock, std::chrono::milliseconds(millsecond),
                            [&] { return !_queue.empty() || hasNotify(lockId); });
    }
  }

  return !_queue.empty();
}

}  // namespace kant
#endif
