// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include "worker/person_tracking_config.h"

#include <iostream>
#include <string>

using TrackingConfig = PT::PersonTrackingConfiguration::TrackingConfiguration;
using SkeletonConfig = PT::SkeletonJointsConfiguration;

PersonTrackingConfig::PersonTrackingConfig() {
  Reset();
}

bool PersonTrackingConfig::GetConfigInfo(PersonTrackerOptions* options) {
  if (option_helper_.options_) {
    option_helper_.options_->ExportTo(*options);
    return true;
  }
  return false;
}

int PersonTrackingConfig::GetResolutionWidth(Resolution res) {
  switch (res) {
    case RESOLUTION_QVGA:
      return 320;

    case RESOLUTION_VGA:
      return 640;

    case RESOLUTION_HD:
      return 1280;

    case RESOLUTION_FULLHD:
      return 1920;

    default:
      throw std::runtime_error("unknown value for resolution");
  }
}

int PersonTrackingConfig::GetResolutionHeight(Resolution res) {
  switch (res) {
    case RESOLUTION_QVGA:
      return 240;

    case RESOLUTION_VGA:
      return 480;

    case RESOLUTION_HD:
      return 720;

    case RESOLUTION_FULLHD:
      return 1080;

    default:
      throw std::runtime_error("unknown value for resolution");
  }
}

void PersonTrackingConfig::Reset() {
  option_helper_.options_.reset();
  // Setup default camera options for PT.
  auto camera_options = new DictionaryCameraOptions();
  camera_options->has_member_color = true;
  camera_options->member_color.has_member_width = true;
  camera_options->member_color.member_width = 640;
  camera_options->member_color.has_member_height = true;
  camera_options->member_color.member_height = 480;
  camera_options->member_color.has_member_frameRate = true;
  camera_options->member_color.member_frameRate = 30;
  camera_options->member_color.has_member_isEnabled = true;
  camera_options->member_color.member_isEnabled = true;

  camera_options->has_member_depth = true;
  camera_options->member_depth.has_member_width = true;
  camera_options->member_depth.member_width = 320;
  camera_options->member_depth.has_member_height = true;
  camera_options->member_depth.member_height = 240;
  camera_options->member_depth.has_member_frameRate = true;
  camera_options->member_depth.member_frameRate = 30;
  camera_options->member_depth.has_member_isEnabled = true;
  camera_options->member_depth.member_isEnabled = true;
  camera_options_.reset(camera_options);
  color_resolution_ = RESOLUTION_QVGA;
  depth_resolution_ = RESOLUTION_QVGA;
}

void PersonTrackingConfig::Dump() {}
