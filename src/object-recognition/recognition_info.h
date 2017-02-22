// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef _RECOGNITION_INFO_H_
#define _RECOGNITION_INFO_H_

#include <node.h>
#include <v8.h>

#include <string>

#include "gen/array_helper.h"
#include "gen/generator_helper.h"

class RecognitionInfo {
 public:
  RecognitionInfo();

  RecognitionInfo(const std::string& label, double probability);

  RecognitionInfo(const RecognitionInfo& rhs);

  ~RecognitionInfo();

  RecognitionInfo& operator = (const RecognitionInfo& rhs);

  void CopyFrom(const RecognitionInfo& rhs);

 public:
  std::string get_label() const {
    return this->label_;
  }

  double get_probability() const {
    return this->probability_;
  }

  void SetJavaScriptThis(v8::Local<v8::Object> obj) {
    // Ignore this if you don't need it
    // Typical usage: emit an event on `obj`
  }

 private:
  std::string label_;

  double probability_;
};

#endif  // _RECOGNITION_INFO_H_
