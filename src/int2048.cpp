#include "int2048.h"

namespace sjtu {

int2048::int2048() : number({0,}), negative{false} {
  ;
}

int2048::~int2048() = default;

int2048::int2048(long long x) : number({static_cast<unsigned long long>(x),}), negative{false} {
  if (int2048::countl_zero(number[0]) == 0) {
    negative = true;
    number[0] = -number[0];
  }
  unsigned long long t;
  while ((t = number.back() / COMPRESS_FACTOR)) {
    number.back() %= COMPRESS_FACTOR;
    number.push_back(t);
  }
}

int2048::int2048(const std::string &s) : negative{false} {
  std::size_t begin = 0;
  if (s[0] == '-') {
    negative = true;
    begin = 1;
  }
  while (begin < s.size() && s[begin] == '0') begin++;
  if (begin == s.size()) {
    negative = false;
    number.resize(1);
    return;
  }
  number.resize((s.size() - begin + COMPRESS_DIGITS - 1) / COMPRESS_DIGITS);
  for (std::size_t i = 0; i < number.size(); i++) {
    for (std::size_t j = begin + COMPRESS_DIGITS * (i + 1) > s.size() ? begin : s.size() - COMPRESS_DIGITS * (i + 1);
          j < s.size() - COMPRESS_DIGITS * i; j++) {
      number[i] = number[i] * 10 + (s[j] ^ '0');
    }
  }
}
int2048::int2048(const int2048 &) = default;
int2048::int2048(int2048 &&) = default;
void int2048::read(const std::string &s) {
  *this = int2048(s);
}
void int2048::print() {
  std::cout << *this;
}

int2048::operator double() const {
  std::stringstream ss;
  ss << *this;
  double ret;
  ss >> ret;
  return ret;
};

int2048 &int2048::add(const int2048 &x) {
  if (number.size() < x.number.size()) number.resize(x.number.size());
  if (negative ^ x.negative) {
    for (std::size_t i = 0; i < x.number.size(); i++) {
      number[i] -= x.number[i];
    }
  } else {
    for (std::size_t i = 0; i < x.number.size(); i++) {
      number[i] += x.number[i];
    }
  }
  while (!number.empty() && number.back() == 0) number.pop_back();
  if (number.empty()) {
    negative = false;
    number.push_back(0);
    return *this;
  }
  if (number.back() > COMPRESS_FACTOR << 1) {
    negative = !negative;
    for (std::size_t i = 0; i < number.size(); i++) number[i] = -number[i];
  }
  for (size_t i = 0; i + 1 < number.size(); i++) {
    number[i + 1] += (number[i] + COMPRESS_FACTOR) / COMPRESS_FACTOR - 1;
    number[i] = (number[i] + COMPRESS_FACTOR) % COMPRESS_FACTOR;
  }
  while (!number.empty() && number.back() == 0) number.pop_back(); // cannot be zero now
  if (auto t = number.back() / COMPRESS_FACTOR; t) {
    number.back() %= COMPRESS_FACTOR;
    number.push_back(t);
  }
  return *this;
}
int2048 &int2048::minus(const int2048 &x) {
  if (number.size() < x.number.size()) number.resize(x.number.size());
  if (negative ^ x.negative) {
    for (std::size_t i = 0; i < x.number.size(); i++) {
      number[i] += x.number[i];
    }
  } else {
    for (std::size_t i = 0; i < x.number.size(); i++) {
      number[i] -= x.number[i];
    }
  }
  while (!number.empty() && number.back() == 0) number.pop_back();
  if (number.empty()) {
    negative = false;
    number.push_back(0);
    return *this;
  }
  if (number.back() > COMPRESS_FACTOR << 1) {
    negative = !negative;
    for (std::size_t i = 0; i < number.size(); i++) number[i] = -number[i];
  }
  for (size_t i = 0; i + 1 < number.size(); i++) {
    number[i + 1] += (number[i] + COMPRESS_FACTOR) / COMPRESS_FACTOR - 1;
    number[i] = (number[i] + COMPRESS_FACTOR) % COMPRESS_FACTOR;
  }
  while (!number.empty() && number.back() == 0) number.pop_back(); // cannot be zero now
  if (auto t = number.back() / COMPRESS_FACTOR; t) {
    number.back() %= COMPRESS_FACTOR;
    number.push_back(t);
  }
  return *this;
}
int2048 add(int2048 x, const int2048 &y) {
  return x.add(y);
}
int2048 minus(int2048 x, const int2048 &y) {
  return x.minus(y);
}

int2048 &int2048::operator=(const int2048 &) = default;
int2048 &int2048::operator=(int2048 &&) = default;

int2048 int2048::operator+() const {
  return *this;
}
int2048 int2048::operator-() const {
  if (*this != 0) {
    int2048 x(*this);
    x.negative = !x.negative;
    return x;
  } else {
    return *this;
  }
}

int2048 &int2048::operator+=(const int2048 &x) {
  return this->add(x);
}
int2048 operator+(int2048 x, const int2048 &y) {
  return x.add(y);
}
int2048 &int2048::operator-=(const int2048 &x) {
  return this->minus(x);
}
int2048 operator-(int2048 x, const int2048 &y) {
  return x.minus(y);
}

unsigned long long int2048::qmul(unsigned long long x, unsigned long long y) { // MAGIC!
  unsigned long long result = x * y - (unsigned long long)((long double) x / MOD * y) * MOD;
  return (result + MOD) % MOD;
}
unsigned long long int2048::qpow(unsigned long long x, unsigned long long b) { // quick power with mod
  unsigned long long ret = 1;
  while (b) {
    if (b & 1) ret = qmul(ret, x);
    x = qmul(x, x);
    b >>= 1;
  }
  return ret;
}

int2048 &int2048::operator*=(const int2048 &x) {
  return *this = *this * x;
}
int2048 operator*(const int2048 &x, const int2048 &y) {
  if (x == 0ll || y == 0ll) return int2048(0);
  int2048 ans;
  ans.negative = x.negative ^ y.negative;
  std::size_t new_size = x.number.size() + y.number.size() - 1;
  ans.number.resize(new_size);
  if (x.number.size() < 15 || y.number.size() < 15) {
    for (std::size_t i = 0; i < x.number.size(); i++) {
      for (std::size_t j = 0; j < y.number.size(); j++) {
        ans.number[i + j] += x.number[i] * y.number[j];
      }
    }
  } else {
    int lg_new_size = (sizeof(std::size_t) * 8 - int2048::countl_zero(new_size - 1)); // 64-bit
    new_size = 1 << lg_new_size;
    ans.number.resize(new_size);
    std::vector<unsigned long long> root(new_size + 1);
    root[0] = 1;
    root[1] = int2048::qpow(int2048::BASIC_ROOT, 1llu << (int2048::BASIC_LG_PERIOD - lg_new_size));
    for (std::size_t i = 2; i <= new_size; i++) {
      root[i] = int2048::qmul(root[i - 1], root[1]);
    }
    std::vector<unsigned long long> trans_x(new_size), trans_y(new_size);
    for(std::size_t i = 0, j = 0; i < x.number.size(); i++) {
      trans_x[j] = x.number[i];
      j ^= new_size - (sizeof(char) << (sizeof(std::size_t) * 8 - int2048::countl_zero(~j & (new_size - 1)) - 1)); // 64-bit
    }
    for(std::size_t i = 0, j = 0; i < y.number.size(); i++) {
      trans_y[j] = y.number[i];
      j ^= new_size - (sizeof(char) << (sizeof(std::size_t) * 8 - int2048::countl_zero(~j & (new_size - 1)) - 1)); // 64-bit
    }
    for (int i = 0; i < lg_new_size; i++) {
      for (std::size_t j = 0; j < new_size; j += (2 << i)) {
        for (std::size_t k = 0; k < (sizeof(char) << i); k++) {
          trans_x[j + k + (1 << i)] = (trans_x[j + k] + int2048::MOD -
            int2048::qmul(trans_x[j + k + (1 << i)], root[k << (lg_new_size - i - 1)])) % int2048::MOD;
          trans_x[j + k] = (trans_x[j + k] * 2 + int2048::MOD - trans_x[j + k + (1 << i)]) % int2048::MOD;
          trans_y[j + k + (1 << i)] = (trans_y[j + k] + int2048::MOD -
            int2048::qmul(trans_y[j + k + (1 << i)], root[k << (lg_new_size - i - 1)])) % int2048::MOD;
          trans_y[j + k] = (trans_y[j + k] * 2 + int2048::MOD - trans_y[j + k + (1 << i)]) % int2048::MOD;
        }
      }
    }
    for(std::size_t i = 0, j = 0; i < new_size; i++) {
      ans.number[j] = int2048::qmul(trans_x[i], trans_y[i]);
      j ^= new_size - (sizeof(char) << (sizeof(std::size_t) * 8 - int2048::countl_zero(~j & (new_size - 1)) - 1)); // 64-bit
    }
    for (int i = 0; i < lg_new_size; i++) {
      for (std::size_t j = 0; j < new_size; j += (2 << i)) {
        for (std::size_t k = 0; k < (sizeof(char) << i); k++) {
          ans.number[j + k + (1 << i)] = (ans.number[j + k] + int2048::MOD -
            int2048::qmul(ans.number[j + k + (1 << i)], root[new_size - (k << (lg_new_size - i - 1))])) % int2048::MOD;
          ans.number[j + k] = (ans.number[j + k] * 2 + int2048::MOD - ans.number[j + k + (1 << i)]) % int2048::MOD;
        }
      }
    }
    auto t = int2048::qpow(new_size, int2048::MOD - 2);
    for (std::size_t i = 0; i < new_size; i++) {
      ans.number[i] = int2048::qmul(ans.number[i], t);
    }
  }
  for (std::size_t i = 0; i + 1 < ans.number.size(); i++) {
    ans.number[i + 1] += ans.number[i] / int2048::COMPRESS_FACTOR;
    ans.number[i] %= int2048::COMPRESS_FACTOR;
  }
  if (auto t = ans.number.back() / int2048::COMPRESS_FACTOR; t) {
    ans.number.back() %= int2048::COMPRESS_FACTOR;
    ans.number.push_back(t);
  }
  while (ans.number.back() == 0) ans.number.pop_back(); // nonzero
  return ans;
}

int2048 operator/(int2048 n, const int2048 &xx) { // O(1) TRY TECH (SHI SHANG YONG YING WEN ZEN ME SHUO A)
  unsigned long long factor = int2048::COMPRESS_FACTOR / (xx.number.back() + 1);
  int2048 x, ans;
  if (factor >= 2) {
    n *= factor;
    x = xx * factor;
  } else {
    x = xx;
  }
  if (n.number.size() < x.number.size() + 1) n.number.resize(x.number.size() + 1);
  else n.number.push_back(0);
  ans.negative = n.negative ^ xx.negative;
  ans.number.resize(n.number.size() - x.number.size());
  for (std::size_t i = n.number.size() - x.number.size() - 1; i != static_cast<std::size_t>(-1); i--) {
    ans.number[i] = (n.number[i + x.number.size()] * int2048::COMPRESS_FACTOR + 
      n.number[i + x.number.size() - 1]) / (x.number.back() + 1);
    int2048 minus(x * ans.number[i]);
    for (std::size_t j = 0; j < minus.number.size(); j++) {
      n.number[i + j] -= minus.number[j];
      if (n.number[i + j] > int2048::COMPRESS_FACTOR) {
        n.number[i + j] += int2048::COMPRESS_FACTOR;
        n.number[i + j + 1]--;
      }
    }
    bool again = n.number[i + x.number.size()];
    if (!again) {
      for (std::size_t j = x.number.size() - 1; ; j--) {
        if (n.number[i + j] > x.number[j]) {
          again = true;
          break;
        } else if (n.number[i + j] < x.number[j]) {
          break;
        } else if (j == 0) {
          again = true;
          break;
        }
      }
    }
    if (again) {
      ans.number[i]++;
      for (std::size_t j = 0; j < x.number.size(); j++) {
        n.number[i + j] -= x.number[j];
        if (n.number[i + j] > int2048::COMPRESS_FACTOR) {
          n.number[i + j] += int2048::COMPRESS_FACTOR;
          n.number[i + j + 1]--;
        }
      }
      again = n.number[i + x.number.size()];
      if (!again) {
        for (std::size_t j = x.number.size() - 1; ; j--) {
          if (n.number[i + j] > x.number[j]) {
            again = true;
            break;
          } else if (n.number[i + j] < x.number[j]) {
            break;
          } else if (j == 0) {
            again = true;
            break;
          }
        }
      }
      if (again) {
        ans.number[i]++;
        for (std::size_t j = 0; j < x.number.size(); j++) {
          n.number[i + j] -= x.number[j];
          if (n.number[i + j] > int2048::COMPRESS_FACTOR) {
            n.number[i + j] += int2048::COMPRESS_FACTOR;
            n.number[i + j + 1]--;
          }
        }
      }
    }
  }
  while (!ans.number.empty() && ans.number.back() == 0) ans.number.pop_back();
  if (ans.number.empty()) {
    ans.negative = false;
    ans.number.push_back(0);
  }
  while (!n.number.empty() && n.number.back() == 0) n.number.pop_back();
  if (n.number.empty() || !(n.negative ^ x.negative)) {
    return ans;
  } else return ans - 1;
}
int2048 &int2048::operator/=(const int2048 &x) {
  return *this = *this / x;
}

int2048 operator%(int2048 x, const int2048 &y) {
  return x %= y;
}
int2048 &int2048::operator%=(const int2048 &x) {
  return *this -= *this / x * x;
}

std::istream &operator>>(std::istream &in, int2048 &x) {
  std::string s;
  in >> s;
  x = int2048(s);
  return in;
}
std::ostream &operator<<(std::ostream &out, const int2048 &x) {
  auto ch = out.fill('0');
  if (x.negative) out.put('-');
  out << x.number.back();
  for (int i = x.number.size() - 2; i >= 0; i--) {
    out.width(x.COMPRESS_DIGITS);
    out << x.number[i];
  }
  out.fill(ch);
  return out;
}

bool operator==(const int2048 &x, const int2048 &y) {
  return x.negative == y.negative && x.number == y.number;
}
bool operator!=(const int2048 &x, const int2048 &y) {
  return x.negative != y.negative || x.number != y.number;
}
bool operator<(const int2048 &x, const int2048 &y) {
  if (x.negative && !y.negative) return true;
  if (!x.negative && y.negative) return false;
  if (!x.negative && !y.negative) {
    if (x.number.size() != y.number.size()) return x.number.size() < y.number.size();
    for(std::size_t i = x.number.size() - 1; i; i--) {
      if (x.number[i] != y.number[i]) return x.number[i] < y.number[i];
    }
    return x.number[0] < y.number[0];
  } else {
    return -y < -x;
  }
}
bool operator>(const int2048 &x, const int2048 &y) {
  return y < x;
}
bool operator<=(const int2048 &x, const int2048 &y) {
  return !(y < x);
}
bool operator>=(const int2048 &x, const int2048 &y) {
  return !(x < y);
}
size_t int2048::countl_zero(unsigned long long x) {
  if (!x) return 64;
  size_t ans = 0;
  static unsigned long long const s[6] = {
    0xffffffff00000000llu, 
    0xffff000000000000llu,
    0xff00000000000000llu,
    0xf000000000000000llu,
    0xc000000000000000llu, 
    0x8000000000000000llu,
  };
  for (int i = 0; i < 6; i++) {
    if (!(x & s[i] >> ans)) {
      ans += 1 << (5 - i);
    }
  }
  return ans;
}
size_t int2048::countl_zero(size_t x) {
  return countl_zero(static_cast<unsigned long long>(x));
}
} // namespace sjtu