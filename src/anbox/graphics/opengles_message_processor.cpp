/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "anbox/logger.h"
#include "anbox/graphics/opengles_message_processor.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/network/connections.h"
#include "anbox/network/delegate_message_processor.h"
#include "anbox/graphics/emugl/RenderThread.h"

#include "external/android-emugl/host/include/libOpenglRender/IOStream.h"

#include <condition_variable>
#include <queue>

namespace {
class DirectIOStream : public IOStream {
public:
    explicit DirectIOStream(const std::shared_ptr<anbox::network::SocketMessenger> &messenger,
                            const size_t &buffer_size = 10000) :
        IOStream(buffer_size),
        messenger_(messenger) {
    }

    virtual ~DirectIOStream() {
        if (send_buffer_ != nullptr) {
            free(send_buffer_);
            send_buffer_ = nullptr;
        }
    }

    void* allocBuffer(size_t min_size) override {
        size_t size = (send_buffer_size_ < min_size ? min_size : send_buffer_size_);
        if (!send_buffer_)
            send_buffer_ = (unsigned char *) malloc(size);
        else if (send_buffer_size_ < size) {
            unsigned char *p = (unsigned char *)realloc(send_buffer_, size);
            if (p != NULL) {
                send_buffer_ = p;
                send_buffer_size_ = size;
            } else {
                free(send_buffer_);
                send_buffer_ = NULL;
                send_buffer_size_ = 0;
            }
        }
        return send_buffer_;
    }

    int commitBuffer(size_t size) override {
        messenger_->send(reinterpret_cast<const char*>(send_buffer_), size);
        return size;
    }

    const unsigned char* readFully(void*, size_t) override {
        ERROR("Not implemented");
        return nullptr;
    }

    const unsigned char* read(void *data, size_t *size) override {
        if (!wait_for_data() || buffer_.size() == 0) {
            *size = 0;
            return nullptr;
        }

        auto bytes_to_read = *size;
        if (bytes_to_read > buffer_.size())
            bytes_to_read = buffer_.size();

        ::memcpy(data, buffer_.data(), bytes_to_read);
        buffer_.erase(buffer_.begin(), buffer_.begin() + bytes_to_read);

        *size = bytes_to_read;

        return static_cast<const unsigned char*>(data);
    }

    int writeFully(const void*, size_t) override {
        ERROR("Not implemented");
        return 0;
    }

    void forceStop() override {
        std::unique_lock<std::mutex> l(mutex_);
        buffer_.clear();
    }

    void submitData(const std::vector<std::uint8_t> &data) {
        std::unique_lock<std::mutex> l(mutex_);
        for (const auto &byte : data)
            buffer_.push_back(byte);
        // buffer_.insert(buffer_.end(), data.begin(), data.end());
        lock_.notify_one();
    }

private:
    bool wait_for_data() {
        std::unique_lock<std::mutex> l(mutex_);

        if (!l.owns_lock())
            return false;

        lock_.wait(l, [&]() { return !buffer_.empty(); });
        return true;
    }

    std::shared_ptr<anbox::network::SocketMessenger> messenger_;
    std::mutex mutex_;
    std::condition_variable lock_;
    std::vector<std::uint8_t> buffer_;
    unsigned char *send_buffer_ = nullptr;
    size_t send_buffer_size_ = 0;
};
}

namespace anbox {
namespace graphics {
emugl::Mutex OpenGlesMessageProcessor::global_lock{};

OpenGlesMessageProcessor::OpenGlesMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    messenger_(messenger),
    stream_(std::make_shared<DirectIOStream>(messenger_)),
    renderer_(RenderThread::create(stream_.get(), &global_lock)) {

    // We have to read the client flags first before we can continue
    // processing the actual commands
    std::array<std::uint8_t, sizeof(unsigned int)> buffer;
    messenger_->receive_msg(boost::asio::buffer(buffer));

    renderer_->start();
}

OpenGlesMessageProcessor::~OpenGlesMessageProcessor() {
    DEBUG("");
    renderer_->forceStop();
    renderer_->wait(nullptr);
}

bool OpenGlesMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
    auto stream = std::static_pointer_cast<DirectIOStream>(stream_);
    stream->submitData(data);
    return true;
}
} // namespace graphics
} // namespace anbox
