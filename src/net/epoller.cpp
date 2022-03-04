#include "src/net/epoller.h"

#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "src/log/logger.h"
#include "src/net/channel.h"

namespace xraft {
namespace net {

namespace {

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

}

EPoller::EPoller(EventLoop* loop)
    : owner_loop_(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_ERROR << "EPoller::EPoller";
    }
}

EPoller::~EPoller() {
    close(epollfd_);
}

util::Timestamp EPoller::poll(int timeout_ms, ChannelList* activate_channels) {
    int events_num = epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
    util::Timestamp now(util::Timestamp::now());
    if (events_num > 0) {
        LOG_INFO << events_num << " events happended";
        fill_activate_channels(events_num, activate_channels);
        if (static_cast<size_t>(events_num) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (events_num == 0) {
        LOG_INFO << "nothing happended";
    } else {
        LOG_ERROR << "EPoller::poll";
    }
    return now;
}

void EPoller::update_channel(Channel* channel) {
    assert_in_loop_thread();
    LOG_INFO << "fd = " << channel->get_fd() << " events = " << channel->get_events();
    const int index = channel->get_index();
    if (index == kNew || index == kDeleted) {
        int fd = channel->get_fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
            // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->get_fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->is_none_event()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::remove_channel(Channel* channel) {
    assert_in_loop_thread();
    int fd = channel->get_fd();
    LOG_INFO << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_none_event());
    int index = channel->get_index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPoller::fill_activate_channels(int event_num, ChannelList* activate_channels) const {
    assert(static_cast<size_t>(event_num) <= events_.size());
    for (int i = 0; i < event_num; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

        int fd = channel->get_fd();
        ChannelMap::const_iterator iter = channels_.find(fd);
        assert(iter != channels_.end());
        assert(iter->second == channel);

        channel->set_revents(events_[i].events);
        activate_channels->push_back(channel);
    }
}

void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->get_events();
    event.data.ptr = channel;
    int fd = channel->get_fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl op=" << operation << " fd=" << fd;
        } else {
            LOG_ERROR << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

}
}