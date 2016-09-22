/*
 * Copyright 2016 <Admobilize>
 */

#ifndef SRC_MALOS_BASE_H_
#define SRC_MALOS_BASE_H_

#include <memory>
#include <mutex>
#include <string>

#include "./zmq_push.h"
#include "./zmq_pull.h"

#include "./src/malos.pb.h"

namespace matrix_malos {

class MalosBase {
 public:
  explicit MalosBase(const std::string &driver_name)
      : base_port_(0),
        is_active_(false),
        provides_updates_(true),
        delay_between_updates_(1000),
        needs_pings_(true),
        timeout_after_last_ping_(5000),
        mandatory_configuration_(false),
        has_been_configured_(false),
        driver_name_(driver_name),
        notes_for_human_("") {}

  // Base port for all the 0MQ  channels.
  // base_port => config channel (pull)
  // base_port + 1 => keepalive channel (pull)
  // base_port + 2 => error channel (push)
  // base_port + 3 => send update channel (push)
  // Bind_scope is the IP to listen on. Use "*" to listen on every interfface.
  bool Init(int base_port, const std::string &bind_scope);

  // Thread that waits for configuration changes.
  void ConfigThread();
  // Thread that send updates to subscribers.
  void UpdateThread();
  // Thead that receives the keepalives.
  void KeepAliveThread();

  // This function should be overridden by drivers. Where is where they send
  // updates to subscribed processes.
  virtual bool SendUpdate() { return false; }

  // Process device-specific configuration present in the protocol buffer
  // of type DriverConfig. Used by derived classes.
  // Check the Everloop driver for an example.
  virtual bool ProcessConfig(const DriverConfig &) { return true; }

  // Does the device provide updates? This is true by default.
  // Derived classes should call this in their constructors if needed.
  void SetProvidesUpdates(bool value) { provides_updates_ = value; }

  // Does the device need keepalive/ping messages? This is true by default.
  // Derived classes should call this in their constructors if needed.
  void SetNeedsKeepalives(bool value) { needs_pings_ = value; }

  // Can the driver send updates when it hasn't been configured?
  void SetMandatoryConfiguration(bool value) {
    mandatory_configuration_ = value;
  }

  // Notes for human. Empty by default. This might be set by derived classes.
  virtual void SetNotesForHuman(const std::string &value) {
    notes_for_human_ = value;
  }

  // Fill out information about the driver.
  void FillOutDriverInfo(DriverInfo *driver_info) const;

 private:
  // Base por of the driver.
  int base_port_;
  // Is the driver active and sending updates?
  bool is_active_;
  // Does the device provide updates?
  bool provides_updates_;
  // Delay between updates in milliseconds.
  int delay_between_updates_;
  // Does the device need pings/keepalive messages?
  bool needs_pings_;
  // Timeout after last ping.
  int timeout_after_last_ping_;
  // Is the driver configuration mandatory? If this is true, pings will be
  // ignored until a succesful configuration is received.
  bool mandatory_configuration_;
  // Has the driver been configured?
  bool has_been_configured_;
  // Driver name.
  std::string driver_name_;
  // ZMQ channel where configuration is received.
  std::unique_ptr<ZmqPuller> zmq_pull_config_;
  // ZMQ channel where keepalives are received.
  std::unique_ptr<ZmqPuller> zmq_pull_keepalive_;
  // What info should the driver present to a human?
  std::string notes_for_human_;
  // Mutex for configuration. Updates and configuration might not be
  // compatible so let's prevent them from being called at the same time.
  std::mutex config_mutex_;

 protected:
  // ZMQ channel where errors are sent.
  std::unique_ptr<ZmqPusher> zmq_push_error_;
  // ZMQ channel where actual data updates are sent.
  std::unique_ptr<ZmqPusher> zqm_push_update_;
};

}  // namespace matrix_malos

#endif  // SRC_MALOS_BASE_H_
