#ifndef __FAKE_DAEMON_HPP__
#define  __FAKE_DAEMON_HPP__

#include <sys/types.h>
#include <sys/wait.h>

#include <urd.hpp>
#include <config.hpp>

struct fake_daemon_cfg {

    fake_daemon_cfg(bool dry_run, uint32_t t = 100) :
        m_dry_run(dry_run),
        m_dry_run_duration(t) { }

    bool m_dry_run = false;
    uint32_t m_dry_run_duration = 0;
};

struct fake_daemon {

    static const norns::config::settings default_cfg;

    fake_daemon();
    ~fake_daemon();
    void configure(const bfs::path& config_file, const fake_daemon_cfg& override_cfg);
    void configure(const bfs::path& config_file, const std::string& alias = "");
    void run();
    int stop();

    pid_t m_pid = 0;
    bool m_running = false;
    norns::urd m_daemon;
    norns::config::settings m_config;
};

#endif /* __FAKE_DAEMON_HPP__ */

