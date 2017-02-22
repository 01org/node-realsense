// Copyright (c) 2017 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef _SLAM_TASK_TYPES_H_
#define _SLAM_TASK_TYPES_H_

#include <memory>
#include <string>

#include "common/task/async_task.h"
#include "utils.h"

class SlamPromiseTask : public PromiseTask {
 public:
  utils::Status result_status;
  v8_value_t GetRejected() override {
    return Nan::New(result_status.message().c_str()).ToLocalChecked();
  }
};

class AsyncTaskPayloadWithRunner : public AsyncTaskPayload {
 public:
  explicit AsyncTaskPayloadWithRunner(SlamRunnerDev* runner)
      : runner_(runner) {}

  SlamRunnerDev* runner() {
    return runner_;
  }
  std::shared_ptr<SlamModuleDev> GetSlamModule() {
    return runner_->slam_module();
  }

 protected:
  SlamRunnerDev* runner_;
};

template<typename T>
class SlamPayload : public AsyncTaskPayloadWithRunner{
 public:
  SlamPayload<T>(SlamRunnerDev* runner, T data)
      : AsyncTaskPayloadWithRunner(runner), data_(data) {}

  T data() { return data_; }
  void set_data(const T& data) {
    data_ = data;
  }

 private:
  T data_;
};

template<>
class SlamPayload<void> : public AsyncTaskPayloadWithRunner {
 public:
  SlamPayload<void>(SlamRunnerDev* runner)
      : AsyncTaskPayloadWithRunner(runner) {}
};

class SlamEventTask : public DirectEventEmitterTask {
 public:
  explicit SlamEventTask(SlamEvent event_type) : event_type_(event_type) {}
  v8_object_t QueryJavaScriptThis() override {
    return Nan::New(*(GetPayload()->runner()->js_this()));
  }

  size_t GetEventCount() override { return 1; }

  bool ShouldPopEvent(size_t event_index) override {
    return GetPayload()->runner()->ShouldPopEvent(event_type_);
  }

  std::string GetEventName(size_t event_index) const {
    return utils::SlamEventToString(event_type_);
  }

  AsyncTaskPayloadWithRunner* GetPayload() {
    return reinterpret_cast<AsyncTaskPayloadWithRunner*>(
        AsyncTask::GetPayload());
  }

 private:
  SlamEvent event_type_;
};

#endif  // _SLAM_TASK_TYPES_H_
