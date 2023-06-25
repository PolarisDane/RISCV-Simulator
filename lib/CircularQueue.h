#ifndef RISCV_SIMULATOR_CIRCULARQUEUE
#define RISCV_SIMULATOR_CIRCULARQUEUE

#include <iostream>
#include <cstdio>

template<class DataType, size_t _size>
class CircularQueue {
private:
  DataType _queue[_size];
  size_t head, tail;
public:
  CircularQueue() {
    head = tail = 0;
  }
  CircularQueue(const CircularQueue<DataType, _size>& other) {
    for (size_t i = 0; i < _size; i++) _queue[i] = other._queue[i];
    head = other.head; tail = other.tail;
  }
  CircularQueue& operator =(const CircularQueue& other) {
    if (this == &other) return (*this);
    for (size_t i = 0; i < _size; i++) _queue[i] = other._queue[i];
    head = other.head; tail = other.tail;
  }
  ~CicularQueue() = default;
  bool empty() {
    return head == tail;
  }
  size_t sub(size_t cur) {
    return (cur + 1) % _size;
  }
  size_t pre(size_t cur) {
    return (cur - 1 + _size) % _size;
  }
  bool full() {
    return sub(tail) == head;
  }
  DataType front() {
    return queue[sub(head)];
  }
  void pop() {
    head = sub(head);
  }
  void popback() {
    tail = pre(tail);
  }
  void push(const DataType& data) {
    queue[tail = sub(tail)] = data;
  }
  void clear() {
    head = tail = 0;
  }
  size_t size() {
    return tail >= head ? tail - head : _size - head + tail + 1;
  }
  DataType& operator [](size_t index) {
    return queue[index];
  }
  const DataType& operator [](size_t index) {
    return queue[index];
  }
  size_t FirstIndex() {
    return head;
  }
  size_t LastIndex() {
    return tail;
  }
};

#endif