/*
 * author : Shuichi TAKANO
 * since  : Sat Jan 12 2019 1:56:3
 */
#ifndef BC85C0A5_7134_139A_17D1_D501102E805C
#define BC85C0A5_7134_139A_17D1_D501102E805C

#include <array>
#include <set>
#include <stdint.h>
#include <string>

namespace sys
{
class JobManager;
class Mutex;
} // namespace sys

namespace io
{

class BTA2DPSourceManager
{
public:
    using Addr = std::array<uint8_t, 6>;

    struct Entry
    {
        std::string name;
        int rssi{};
        Addr addr;

        friend bool operator<(const Entry& a, const Entry& b);
    };

    using EntryContainer = std::set<Entry>;

public:
    void initialize(sys::JobManager* jm);

    void startDiscovery(int seconds);
    void stopDiscovery();

    void connect(const Addr& addr);
    void cancelConnect();

    bool isDiscovering() const;
    bool isConnected() const;

    sys::Mutex& getMutex();
    const EntryContainer& getEntries() const;

    static BTA2DPSourceManager& instance();
};

} // namespace io

#endif /* BC85C0A5_7134_139A_17D1_D501102E805C */
