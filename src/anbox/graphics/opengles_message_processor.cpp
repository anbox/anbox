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
#include <functional>

namespace {
constexpr const size_t default_buffer_size{384};
constexpr const size_t max_send_buffer_size{1024};

class DelayedIOStream : public IOStream {
public:
    typedef std::vector<char> Buffer;

    explicit DelayedIOStream(const std::shared_ptr<anbox::network::SocketMessenger> &messenger,
                             size_t buffer_size = default_buffer_size) :
        IOStream(buffer_size),
        messenger_(messenger) {
        // writer_thread_(std::bind(&DelayedIOStream::worker_thread, this)) {
    }

    virtual ~DelayedIOStream() {
        DEBUG("");
        forceStop();
        DEBUG("Shutting down");
    }

    void* allocBuffer(size_t min_size) override {
        DEBUG("min size %d", min_size);
        if (buffer_.size() < min_size)
            buffer_.resize(min_size);
        return buffer_.data();
    }

    int commitBuffer(size_t size) override {
        DEBUG("size %d", size);
        std::unique_lock<std::mutex> l(read_mutex_);

#if 0
        if (buffer_.capacity() <= 2 * size) {
            buffer_.resize(size);
            out_queue_.push(std::move(buffer_));
        } else {
            out_queue_.push(Buffer(buffer_.data(), buffer_.data() + size));
        }
        DEBUG("Submitted data into output queue (%d bytes)", size);
        can_write_.notify_all();
#else
        ssize_t bytes_left = size;
        while (bytes_left > 0) {
            const ssize_t written = messenger_->send_raw(buffer_.data() + (size - bytes_left), bytes_left);
            if (written < 0 ) {
                if (errno != EINTR) {
                    ERROR("Failed to write data: %s", std::strerror(errno));
                    break;
                }
                WARNING("Socket busy, trying again");
            } else
                bytes_left -= written;
        }

        DEBUG("Sent data to remote (%d bytes)", buffer_.size());
#endif
        return static_cast<int>(size);
    }

    const unsigned char* readFully(void *buffer, size_t length) override {
        size_t size = length;
        auto data = read(buffer, &size);
        if (size < length)
            return nullptr;
        return data;
    }

    const unsigned char* read(void *buffer, size_t *length) override {
        std::unique_lock<std::mutex> l(read_mutex_);

        if (stopped_) {
            DEBUG("Aborting");
            return nullptr;
        }

        if (current_read_buffer_left_ == 0 && in_queue_.empty()) {
            DEBUG("Waiting for data to be available");
            can_read_.wait(l);

            if (stopped_) {
                DEBUG("Aborting");
                return nullptr;
            }
        }

        DEBUG("Trying to read %d bytes", *length);
        size_t read = 0;
        auto buf = static_cast<unsigned char*>(buffer);
        const auto buffer_end = buf + *length;
        while (buf != buffer_end) {
            if (current_read_buffer_left_ == 0) {
                // If we don't have anymore buffers we need to stop reading here
                if (in_queue_.empty())
                    break;

                current_read_buffer_ = in_queue_.front();
                in_queue_.pop();
                current_read_buffer_left_ = current_read_buffer_.size();
            }

            const size_t current_size = std::min<size_t>(buffer_end - buf,
                                                         current_read_buffer_left_);
            ::memcpy(buffer, current_read_buffer_.data() +
                     (current_read_buffer_.size() - current_read_buffer_left_),
                     current_size);

            read += current_size;
            buf += current_size;
            current_read_buffer_left_ -= current_size;

            DEBUG("Size %d, left to read %d", current_size, current_read_buffer_left_);
        }

        if (read == 0)
            return nullptr;

        *length = read;

        DEBUG("Read %d bytes (buffers left %d)", read, in_queue_.size());

        return buf;
    }

    int writeFully(const void *buffer, size_t length) override {
        (void) buffer;
        (void) length;
        ERROR("Not implemented");
        return -1;
    }

    void forceStop() override {
        DEBUG("");
        stopped_ = true;
        can_read_.notify_all();
        can_write_.notify_all();
    }

    void post_data(const Buffer &buffer) {
        DEBUG("Got data and waiting for lock");
        std::unique_lock<std::mutex> l(read_mutex_);
        DEBUG("Received %d bytes", buffer.size());
        in_queue_.push(std::move(buffer));
        can_read_.notify_all();
    }

private:
    void worker_thread() {
        DEBUG("Running send thread");
        while (true) {
            std::unique_lock<std::mutex> l(write_mutex_);
            while (out_queue_.empty() && !stopped_) {
                can_write_.wait(l, [&]() { return !out_queue_.empty() || stopped_; });
                DEBUG("Woke up (queue size %d)", out_queue_.size());
            }

            if (stopped_)
                break;

            DEBUG("Going to send out %d bytes", out_queue_.front().size());

            auto buffer = out_queue_.front();
            out_queue_.pop();

            ssize_t bytes_left = buffer.size();
            while (bytes_left > 0) {
                const ssize_t written = messenger_->send_raw(buffer.data() + (buffer.size() - bytes_left), bytes_left);
                if (written < 0 ) {
                    if (errno != EINTR) {
                        ERROR("Failed to write data: %s", std::strerror(errno));
                        break;
                    }
                    WARNING("Socket busy, trying again");
                } else
                    bytes_left -= written;
            }
            DEBUG("Sent %d bytes to client (queue size %d)", buffer.size(), out_queue_.size());
        }

        DEBUG("Shutting down");
    }

    std::shared_ptr<anbox::network::SocketMessenger> messenger_;
    Buffer buffer_;
    std::thread writer_thread_;
    std::queue<Buffer> out_queue_;
    Buffer current_write_buffer_;
    size_t current_write_buffer_left_ = 0;
    std::queue<Buffer> in_queue_;
    Buffer current_read_buffer_;
    size_t current_read_buffer_left_ = 0;
    std::mutex write_mutex_;
    std::mutex read_mutex_;
    std::condition_variable can_write_;
    std::condition_variable can_read_;
    bool stopped_ = false;
};
}

namespace anbox {
namespace graphics {
emugl::Mutex OpenGlesMessageProcessor::global_lock{};
static int next_id = 0;
OpenGlesMessageProcessor::OpenGlesMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    messenger_(messenger),
    id_(next_id++),
    stream_(std::make_shared<DelayedIOStream>(messenger_)) {

    // We have to read the client flags first before we can continue
    // processing the actual commands
    unsigned int client_flags = 0;
    auto err = messenger_->receive_msg(boost::asio::buffer(&client_flags, sizeof(unsigned int)));
    if (err)
        ERROR("%s", err.message());

    renderer_.reset(RenderThread::create(stream_.get(), &global_lock));
    renderer_->start();

    DEBUG("Started new OpenGL ES message processor");
}

OpenGlesMessageProcessor::~OpenGlesMessageProcessor() {
    DEBUG("");
    renderer_->forceStop();
    renderer_->wait(nullptr);
}

bool OpenGlesMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
    DEBUG("[%d] Got %d bytes", id_, data.size());
    auto stream = std::static_pointer_cast<DelayedIOStream>(stream_);
    stream->post_data(DelayedIOStream::Buffer(data.data(), data.data() + data.size()));
    return true;
}
} // namespace graphics
} // namespace anbox
