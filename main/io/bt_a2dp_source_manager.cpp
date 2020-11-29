/*
 * author : Shuichi TAKANO
 * since  : Sat Jan 12 2019 1:58:2
 */

#include "bt_a2dp_source_manager.h"
#include <array>
#include <audio/audio_out.h>
#include <debug.h>
#include <esp_a2dp_api.h>
#include <esp_avrc_api.h>
#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <math.h>
#include <mutex>
#include <string.h>
#include <system/job_manager.h>
#include <system/mutex.h>

namespace io
{

struct Impl : public audio::AudioOutDriver
{
    enum class State
    {
        IDLE,
        DISCOVERING,
        DISCOVERED,
        UNCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
    };

    enum class MediaState
    {
        IDLE,
        STARTING,
        STARTED,
        STOPPING,
    };

    State state_{};
    MediaState mediaState_{};

    using Addr = BTA2DPSourceManager::Addr;
    Addr addr_;
    //    std::string deviceName_;

    sys::JobManager* jobManager_{};
    TimerHandle_t timerHandle_;

    int connectingInterval_ = 0;
    int mediaInterval_      = 0;

    float volume_ = 1.0f;

    sys::Mutex mutex_;
    BTA2DPSourceManager::EntryContainer entries_;

public:
    void connect()
    {
        DBOUT(("connecting to peer\n"));
        state_ = State::CONNECTING;
        esp_a2d_source_connect(addr_.data());
    }

    void onGAPEvent(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param)
    {
        std::lock_guard<sys::Mutex> lock(mutex_);

        switch (event)
        {

        case ESP_BT_GAP_DISC_RES_EVT:
        {
            auto& p = param->disc_res;
            DBOUT(("discovered device: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   p.bda[0],
                   p.bda[1],
                   p.bda[2],
                   p.bda[3],
                   p.bda[4],
                   p.bda[5]));

            std::string name;
            int rssi = -65536;
            std::array<uint8_t, 6> addr;
            memcpy(addr.data(), p.bda, 6);

            for (int i = 0; i < p.num_prop; i++)
            {
                auto& prop = p.prop[i];
                switch (prop.type)
                {
                case ESP_BT_GAP_DEV_PROP_COD:
                {
                    auto cod = *(uint32_t*)prop.val;
                    DBOUT(("  Class of Device: 0x%x\n", cod));
                    if (!esp_bt_gap_is_valid_cod(cod) ||
                        !(esp_bt_gap_get_cod_srvc(cod) &
                          ESP_BT_COD_SRVC_RENDERING))
                    {
                        return;
                    }
                    break;
                }
                case ESP_BT_GAP_DEV_PROP_RSSI:
                {
                    rssi = *(int8_t*)prop.val;
                    DBOUT(("  RSSI: %d\n", rssi));
                    break;
                }
                case ESP_BT_GAP_DEV_PROP_EIR:
                {
                    auto eir = (uint8_t*)prop.val;

                    uint8_t l;
                    auto nameData = esp_bt_gap_resolve_eir_data(
                        eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &l);
                    if (!nameData)
                    {
                        nameData = esp_bt_gap_resolve_eir_data(
                            eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &l);
                    }

                    name = std::string((char*)nameData, (char*)nameData + l);
                    DBOUT(("  name: '%s'\n", name.c_str()));
                    break;
                }

                case ESP_BT_GAP_DEV_PROP_BDNAME:
                default:
                    break;
                }
            }

            if (!name.empty())
            {
                std::lock_guard<sys::Mutex> lock(mutex_);
                entries_.insert({name, rssi, addr});
            }

            if (0)
            {
                //                    if (name != "TaoTronics TT-BR06")
                //                    if (name != "JBL GO")
                if (name.empty())
                {
                    return;
                }

                state_ = State::DISCOVERED;
                memcpy(addr_.data(), p.bda, 6);
                //                    deviceName_ = name;

                esp_bt_gap_cancel_discovery();
            }

            break;
        }

        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        {
            auto& p = param->disc_st_chg;
            if (p.state == ESP_BT_GAP_DISCOVERY_STOPPED)
            {
                DBOUT(("Device discovery stopped.\n"));
                if (state_ == State::DISCOVERED)
                {
                    connect();
                }
                else if (state_ == State::DISCOVERING)
                {
                    DBOUT(
                        ("Device discovery failed, continue to discover...\n"));
                    esp_bt_gap_start_discovery(
                        ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
                }
            }
            else if (p.state == ESP_BT_GAP_DISCOVERY_STARTED)
            {
                DBOUT(("Discovery started.\n"));
                state_ = State::DISCOVERING;
            }
            break;
        }

        case ESP_BT_GAP_RMT_SRVCS_EVT:
        case ESP_BT_GAP_RMT_SRVC_REC_EVT:
            break;

        case ESP_BT_GAP_AUTH_CMPL_EVT:
        {
            auto& p = param->auth_cmpl;
            if (p.stat == ESP_BT_STATUS_SUCCESS)
            {
                DBOUT(("authentication success: %s\n", p.device_name));
            }
            else
            {
                DBOUT(("authentication failed, status:%d\n", p.stat));
            }
            break;
        }

        case ESP_BT_GAP_PIN_REQ_EVT:
        {
            auto& p = param->pin_req;
            DBOUT(("ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d\n", p.min_16_digit));
            esp_bt_pin_code_t pin = {0};
            int len               = 0;
            if (p.min_16_digit)
            {
                DBOUT(("Input pin code: 0000 0000 0000 0000\n"));
                len = 16;
            }
            else
            {
                DBOUT(("Input pin code: 1234\n"));
                pin[0] = '1';
                pin[1] = '2';
                pin[2] = '3';
                pin[3] = '4';
                len    = 4;
            }
            esp_bt_gap_pin_reply(p.bda, true, len, pin);
            break;
        }

#if CONFIG_BT_SSP_ENABLED
        case ESP_BT_GAP_CFM_REQ_EVT:
        {
            auto& p = param->cfm_req;
            DBOUT(("Please compare the numeric value: %d\n", p.num_val));
            esp_bt_gap_ssp_confirm_reply(p.bda, true);
            break;
        }

        case ESP_BT_GAP_KEY_NOTIF_EVT:
        {
            auto& p = param->key_notif;
            DBOUT(("passkey:%d\n", p.passkey));
            break;
        }

        case ESP_BT_GAP_KEY_REQ_EVT:
            DBOUT(("Please enter passkey!\n"));
            break;
#endif

        default:
        {
            DBOUT(("unhandled GAP event: %d", event));
            break;
        }
        }
    }

    void updateMediaState(const esp_a2d_cb_param_t* param)
    {
        switch (mediaState_)
        {
        case MediaState::IDLE:
        {
            if (param)
            {
                const auto& p = param->media_ctrl_stat;
                if (p.cmd == ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY &&
                    p.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
                {
                    DBOUT(("a2dp media ready, starting ...\n"));
                    esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
                    mediaState_ = MediaState::STARTING;
                }
            }
            else
            {
                // heart beat
                DBOUT(("a2dp media ready checking ..."));
                esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY);
            }
            break;
        }

        case MediaState::STARTING:
        {
            if (param)
            {
                const auto& p = param->media_ctrl_stat;
                if (p.cmd == ESP_A2D_MEDIA_CTRL_START &&
                    p.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
                {
                    DBOUT(("a2dp media start successfully.\n"));
                    mediaInterval_ = 0;
                    mediaState_    = MediaState::STARTED;
                }
                else
                {
                    DBOUT(("a2dp media start failed.\n"));
                    mediaState_ = MediaState::IDLE;
                }
            }
            break;
        }

        case MediaState::STARTED:
        {
#if 0
            if (!param)
            {
                if (++mediaInterval_ >= 10)
                {
                    DBOUT(("a2dp media stopping...\n"));
                    esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
                    mediaState_    = MediaState::STOPPING;
                    mediaInterval_ = 0;
                }
            }
#endif
            break;
        }

        case MediaState::STOPPING:
        {
            if (param)
            {
                const auto& p = param->media_ctrl_stat;
                if (p.cmd == ESP_A2D_MEDIA_CTRL_STOP &&
                    p.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
                {
                    DBOUT((
                        "a2dp media stopped successfully, disconnecting...\n"));
                    mediaState_ = MediaState::IDLE;
                    esp_a2d_source_disconnect(addr_.data());
                    state_ = State::DISCONNECTING;
                }
                else
                {
                    DBOUT(("a2dp media stopping...\n"));
                    esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
                }
            }
            break;
        }
        }
    }

    void onA2DPEvent(esp_a2d_cb_event_t event, const esp_a2d_cb_param_t* param)
    {
        std::lock_guard<sys::Mutex> lock(mutex_);

        DBOUT(("onA2DPEvent: state %d, mediaState %d, ev %d\n",
               (int)state_,
               (int)mediaState_,
               event));

        switch (event)
        {
        case ESP_A2D_CONNECTION_STATE_EVT:
        {
            auto& p = param->conn_stat;
            if (p.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
            {
                memcpy(addr_.data(), p.remote_bda, 6);
                DBOUT(("a2dp connected: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       addr_[0],
                       addr_[1],
                       addr_[2],
                       addr_[3],
                       addr_[4],
                       addr_[5]));
                state_      = State::CONNECTED;
                mediaState_ = MediaState::IDLE;
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE,
                                         ESP_BT_NON_DISCOVERABLE);
                esp_bt_gap_cancel_discovery();

                audio::AudioOutDriverManager::instance().setDriver(this);
            }
            else if (p.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
            {
                DBOUT(("a2dp disconnected\n"));
                state_ = State::UNCONNECTED;
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE,
                                         ESP_BT_GENERAL_DISCOVERABLE);
                // discoverに戻ってもいいんじゃ？

                audio::AudioOutDriverManager::instance().setDriver(nullptr);
            }
            break;
        }

        case ESP_A2D_AUDIO_STATE_EVT:
        {
            auto& p = param->audio_stat;
            if (p.state == ESP_A2D_AUDIO_STATE_STARTED)
            {
                DBOUT(("a2d audio state started.\n"));
            }
            break;
        }

        case ESP_A2D_AUDIO_CFG_EVT:
            DBOUT(("A2DP audio config event.\n"));
            break;

        case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        {
            updateMediaState(param);
            break;
        }

        default:
            break;
        }
    }

    void onHeartBeat()
    {
        DBOUT(("onHeartBeat: state %d\n", (int)state_));

        switch (state_)
        {
            // case State::UNCONNECTED:
            // {
            //     esp_a2d_source_connect(addr_.data());
            //     state_              = State::CONNECTING;
            //     connectingInterval_ = 0;
            //     break;
            // }

            // case State::CONNECTING:
            // {
            //     if (++connectingInterval_ >= 2)
            //     {
            //         state_              = State::UNCONNECTED;
            //         connectingInterval_ = 0;
            //     }
            //     break;
            // }

        case State::CONNECTED:
            updateMediaState(nullptr);
            break;

            // case State::DISCONNECTING:
            //     break;

        default:
            break;
        }
    }

    void enterHeartBeat()
    {
        if (state_ != State::IDLE)
        {
            jobManager_->add([this] { onHeartBeat(); });
        }
    }

    size_t updateSampleData(int16_t* data, size_t nSamples)
    {
//        DBOUT(("update sample %zd\n", nSamples));
#if 0
        for (size_t i = 0; i < nSamples; ++i)
        {
            static float phase = 0;
            phase += 440 * 3.14159f * 2 / 44100;

            if (phase > 3.14159f * 2)
            {
                phase -= 3.14159f * 2;
            }

            float v = sinf(phase) * 1024.0f;
            int iv  = (int)v;

            data[0] = iv;
            data[1] = iv;
            data += 2;
        }
        return nSamples;
#else
        auto& audioOutMan = audio::AudioOutDriverManager::instance();
        if (!audioOutMan.lock(this))
        {
            return 0;
        }
        auto scale = int(volume_ * 256);
        for (auto ct = nSamples; ct;)
        {
            auto n = audioOutMan.generateSamples(ct);
            ct -= n;
            auto samples = audioOutMan.getSampleBuffer();
            while (n)
            {
                data[0] = (*samples)[0] * scale >> 16;
                data[1] = (*samples)[1] * scale >> 16;
                data += 2;
                ++samples;
                --n;
            }
        }
        audioOutMan.unlock();
        return nSamples;
#endif
    }

    // AudioOutDriver
    bool isDriverUseUpdate() const override { return false; };
    void onAttach() override{};
    void onDetach() override{};
    uint32_t getSampleRate() const override { return 44100; };
    void setVolume(float v) override { volume_ = v; };
    float getVolume() const override { return volume_; };
};
Impl pimpl_;

void
BTA2DPSourceManager::initialize(sys::JobManager* jm)
{
    pimpl_.jobManager_ = jm;

    jm->add([] {
        esp_bt_gap_register_callback(
            [](esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
                pimpl_.onGAPEvent(event, param);
            });

        esp_a2d_register_callback(
            [](esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param) {
                auto& p = *param;
                pimpl_.jobManager_->add([=] { pimpl_.onA2DPEvent(event, &p); });
            });

        esp_a2d_source_register_data_callback(
            [](uint8_t* data, int32_t len) -> int32_t {
                if (len <= 0)
                {
                    return 0;
                }
                auto n = pimpl_.updateSampleData((int16_t*)data, len >> 2);
                return n << 2;
            });

#if CONFIG_BT_SSP_ENABLED
        esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
        esp_bt_io_cap_t iocap        = ESP_BT_IO_CAP_IO;
        esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif
        esp_a2d_source_init();

        pimpl_.timerHandle_ =
            xTimerCreate("a2dpTimer",
                         (10000 / portTICK_RATE_MS),
                         pdTRUE,
                         nullptr,
                         [](void* p) { pimpl_.enterHeartBeat(); });
        xTimerStart(pimpl_.timerHandle_, portMAX_DELAY);
    });
}

void
BTA2DPSourceManager::startDiscovery(int seconds)
{
    // todo: -1 がきたら継続で discovery する
    std::lock_guard<sys::Mutex> lock(getMutex());

    DBOUT(("Starting device discovery...\n"));
    pimpl_.entries_.clear();
    pimpl_.state_ = Impl::State::DISCOVERING;
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    int len = std::max(0x1, std::min(0x30, (seconds * 100 + 50) >> 7));
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, len, 0);
}

void
BTA2DPSourceManager::stopDiscovery()
{
    std::lock_guard<sys::Mutex> lock(getMutex());

    if (pimpl_.state_ == Impl::State::DISCOVERING)
    {
        DBOUT(("Stop device discovery...\n"));
        pimpl_.state_ = Impl::State::IDLE;
        esp_bt_gap_cancel_discovery();
    }
}

void
BTA2DPSourceManager::connect(const Addr& addr)
{
    std::lock_guard<sys::Mutex> lock(getMutex());

    DBOUT(("Connecting A2DP...\n"));
    pimpl_.addr_ = addr;

    if (pimpl_.state_ == Impl::State::DISCOVERING)
    {
        pimpl_.state_ = Impl::State::DISCOVERED;
        esp_bt_gap_cancel_discovery();
    }
    else
    {
        pimpl_.connect();
    }
}

void
BTA2DPSourceManager::cancelConnect()
{
    DBOUT(("cancel connect"));
    // といってもできることはない？？
}

bool
BTA2DPSourceManager::isDiscovering() const
{
    return pimpl_.state_ == Impl::State::DISCOVERING;
}

bool
BTA2DPSourceManager::isConnected() const
{
    return pimpl_.state_ == Impl::State::CONNECTED;
}

sys::Mutex&
BTA2DPSourceManager::getMutex()
{
    return pimpl_.mutex_;
}

const BTA2DPSourceManager::EntryContainer&
BTA2DPSourceManager::getEntries() const
{
    return pimpl_.entries_;
}

BTA2DPSourceManager&
BTA2DPSourceManager::instance()
{
    static BTA2DPSourceManager inst;
    return inst;
}

//
bool
operator<(const BTA2DPSourceManager::Entry& a,
          const BTA2DPSourceManager::Entry& b)
{
    if (a.name != b.name)
    {
        return a.name < b.name;
    }
    if (a.rssi < b.rssi)
    {
        return a.rssi > b.rssi;
    }
    return a.addr < b.addr;
}

} // namespace io
