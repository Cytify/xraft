#ifndef NONCOPYABLE_H_
#define NONCOPYABLE_H_

namespace util {

class Noncopyable {
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

private:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;
};

}

#endif