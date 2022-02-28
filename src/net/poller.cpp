#include "src/net/poller.h"

#include <assert.h>

#include "src/net/channel.h"
#include "src/log/logger.h"

namespace xraft {
namespace net {

Poller::Poller(EventLoop* loop) : owner_loop_(loop) {

}

Poller::~Poller() {

}

util::Timestamp Poller::poll(int timeout_ms, ChannelList* activate_channels) {
    int event_num = ::poll(&*poll_fds_.begin(), poll_fds_.size(), timeout_ms);
    util::Timestamp now(util::Timestamp::now());
    if (event_num > 0) {
        LOG_INFO << event_num << " events happended";
        fill_activate_channels(event_num, activate_channels);
    } else if (event_num == 0) {
        LOG_INFO << "nothing happended";
    } else {
        LOG_ERROR << "Poller::poll()";
    }

    return now;
}

void Poller::update_channel(Channel* channel) {
    assert_in_loop_thread();
    LOG_INFO << "fd = " << channel->get_fd() << " events = " << channel->get_events();
    if (channel->get_index() < 0) {
        // 新的channel
        assert(channels_.find(channel->get_fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->get_fd();
        pfd.events = static_cast<short>(channel->get_events());
        pfd.revents = 0;
        poll_fds_.push_back(pfd);

        int index = static_cast<int>(poll_fds_.size()) - 1;
        channel->set_index(index);
        channels_[pfd.fd] = channel;
    } else {
        // 已有channel，更新即可
        assert(channels_.find(channel->get_fd()) != channels_.end());
        assert(channels_[channel->get_fd()] == channel);
        int index = channel->get_index();
        assert(index >= 0 && index < static_cast<int>(poll_fds_.size()));
        struct pollfd& pfd = poll_fds_[index];
        assert(pfd.fd == channel->get_fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->get_events());
        pfd.revents = 0;
        if (channel->is_none_event()) {
            pfd.fd = -1;
        }
    }
}

void Poller::fill_activate_channels(int event_num, ChannelList* activate_channels) const {
    for (PollFdList::const_iterator iter = poll_fds_.begin(), end = poll_fds_.end();
         iter != end && event_num > 0; ++iter) {
        if (iter->revents > 0) {
            --event_num;
            auto pos = channels_.find(iter->fd);
            assert(pos != channels_.end());
            Channel* channel = pos->second;
            assert(channel->get_fd() == iter->fd);
            channel->set_revents(iter->revents);
            activate_channels->push_back(channel);
        }
    }
}

}
}