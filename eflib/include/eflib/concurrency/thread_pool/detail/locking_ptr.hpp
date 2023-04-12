#pragma once

namespace eflib {
namespace threadpool {
namespace detail {

/*! \brief  Smart pointer with a scoped locking mechanism.
 *
 * This class is a wrapper for a volatile pointer. It enables synchronized access to the
 * internal pointer by locking the passed mutex.
 */
template <typename T, typename Mutex>
class locking_ptr {
  T* m_obj;        //!< The instance pointer.
  Mutex& m_mutex;  //!< Mutex is used for scoped locking.

  locking_ptr(locking_ptr const&) = delete;

public:
  /// Constructor.
  locking_ptr(volatile T& obj, const volatile Mutex& mtx)
    : m_obj(const_cast<T*>(&obj))
    , m_mutex(*const_cast<Mutex*>(&mtx)) {
    // Lock mutex
    m_mutex.lock();
  }

  /// Destructor.
  ~locking_ptr() {
    // Unlock mutex
    m_mutex.unlock();
  }

  /*! Returns a reference to the stored instance.
   * \return The instance's reference.
   */
  T& operator*() const { return *m_obj; }

  /*! Returns a pointer to the stored instance.
   * \return The instance's pointer.
   */
  T* operator->() const { return m_obj; }
};

}  // namespace detail
}  // namespace threadpool
}  // namespace eflib
