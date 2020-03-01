#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

namespace cxxbridge01 {}
namespace cxxbridge = cxxbridge01;

namespace cxxbridge01 {

class String final {
public:
  String() noexcept;
  String(const String &other) noexcept;
  String(String &&other) noexcept;
  String(const char *s);
  String(const std::string &s);
  String &operator=(const String &other) noexcept;
  String &operator=(String &&other) noexcept;
  ~String() noexcept;
  operator std::string() const;

  // Note: no null terminator.
  const char *data() const noexcept;
  size_t size() const noexcept;
  size_t length() const noexcept;

private:
  // Size and alignment statically verified by rust_string.rs.
  std::array<uintptr_t, 3> repr;
};

class RustStr final {
public:
  RustStr() noexcept;
  RustStr(const char *s);
  RustStr(const std::string &s);
  RustStr(std::string &&s) = delete;
  RustStr(const RustStr &other) noexcept;
  RustStr &operator=(RustStr other) noexcept;
  operator std::string() const;

  // Note: no null terminator.
  const char *data() const noexcept;
  size_t size() const noexcept;
  size_t length() const noexcept;

  // Repr is PRIVATE; must not be used other than by our generated code.
  //
  // Not necessarily ABI compatible with &str. Codegen will translate to
  // cxx::rust_str::RustStr which matches this layout.
  struct Repr {
    const char *ptr;
    size_t len;
  };
  RustStr(Repr repr) noexcept;
  operator Repr() noexcept;

private:
  Repr repr;
};

#ifndef CXXBRIDGE01_RUST_BOX
#define CXXBRIDGE01_RUST_BOX
template <typename T> class RustBox final {
public:
  RustBox(const RustBox &other) : RustBox(*other) {}
  RustBox(RustBox &&other) noexcept : repr(other.repr) { other.repr = 0; }
  RustBox(const T &val) {
    this->uninit();
    ::new (this->deref_mut()) T(val);
  }
  RustBox &operator=(const RustBox &other) {
    if (this != &other) {
      if (this->repr) {
        **this = *other;
      } else {
        this->uninit();
        ::new (this->deref_mut()) T(*other);
      }
    }
    return *this;
  }
  RustBox &operator=(RustBox &&other) noexcept {
    if (this->repr) {
      this->drop();
    }
    this->repr = other.repr;
    other.repr = 0;
    return *this;
  }
  ~RustBox() noexcept {
    if (this->repr) {
      this->drop();
    }
  }

  const T *operator->() const noexcept { return this->deref(); }
  const T &operator*() const noexcept { return *this->deref(); }
  T *operator->() noexcept { return this->deref_mut(); }
  T &operator*() noexcept { return *this->deref_mut(); }

  // Important: requires that `raw` came from an into_raw call. Do not pass a
  // pointer from `new` or any other source.
  static RustBox from_raw(T *raw) noexcept {
    RustBox box;
    box.set_raw(raw);
    return box;
  }

  T *into_raw() noexcept {
    T *raw = this->deref_mut();
    this->repr = 0;
    return raw;
  }

private:
  RustBox() noexcept {}
  void uninit() noexcept;
  void set_raw(T *) noexcept;
  T *get_raw() noexcept;
  void drop() noexcept;
  const T *deref() const noexcept;
  T *deref_mut() noexcept;
  uintptr_t repr;
};
#endif // CXXBRIDGE01_RUST_BOX

std::ostream &operator<<(std::ostream &os, const String &s);
std::ostream &operator<<(std::ostream &os, const RustStr &s);

} // namespace cxxbridge01
