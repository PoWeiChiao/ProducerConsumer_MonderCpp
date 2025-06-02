#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

struct Data {
    int value;
    explicit Data(int i) : value(i) {}
}

std::queue<std::shared_ptr<Data>> buffer;
const size_t MAX_BUFFER_SIZE = 10;
std::mutex mtx;
std::condition_variable bufferNotFull, bufferNotEmpty;

void Producer() {
    int i = 0;
    while (true) {
        auto data = std::make_shared<Data>(i);
        std::unique_lock<std::mutex> lock(mtx);
        bufferNotFull.wait(lock, [] { return buffer.size < MAX_BUFFER_SIZE; });
        buffer.push(data);
        std::cout << "[Producer] " << data->value << std::endl;
        lock.unlock();
        bufferNotEmpty.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        bufferNotEmpty.wait(lock, [] { return !buffer.empty; });
        auto data = buffer.front();
        buffer.pop();
        std::cout << "[Consumer] " << data->value << std::endl;
        lock.unlock();
        bufferNotFull.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::thread pThread(Producer);
    std::thread cThread(Consumer);
    pThread.join();
    cThread.join();
    return 0;
}