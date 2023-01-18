#include "sxt/execution/async/computation_handle.h"

#include <cassert>

#include "sxt/base/device/synchronization.h"
#include "sxt/execution/base/stream.h"
#include "sxt/execution/base/stream_handle.h"
#include "sxt/execution/base/stream_pool.h"

namespace sxt::xena {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
computation_handle::computation_handle(computation_handle&& other) noexcept {
  head_ = other.head_;
  other.head_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
// computation_handle
//--------------------------------------------------------------------------------------------------
computation_handle::~computation_handle() noexcept { this->wait(); }

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
computation_handle& computation_handle::operator=(computation_handle&& other) noexcept {
  this->wait();
  head_ = other.head_;
  other.head_ = nullptr;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// wait
//--------------------------------------------------------------------------------------------------
void computation_handle::wait() noexcept {
  auto pool = xenb::get_stream_pool();
  while (head_ != nullptr) {
    auto handle = head_;
    assert(handle->stream != nullptr);
    basdv::synchronize_stream(handle->stream);
    head_ = handle->next;
    handle->next = nullptr;
    pool->release_handle(handle);
  }
}

//--------------------------------------------------------------------------------------------------
// add_stream
//--------------------------------------------------------------------------------------------------
void computation_handle::add_stream(xenb::stream&& stream) noexcept {
  auto handle = stream.release_handle();
  assert(handle != nullptr && handle->next == nullptr);
  handle->next = head_;
  head_ = handle;
}
} // namespace sxt::xena
